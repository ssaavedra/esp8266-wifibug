# Trying SSL with bottle
# ie combo of http://www.piware.de/2011/01/creating-an-https-server-in-python/
# and http://dgtool.blogspot.com/2011/12/ssl-encryption-in-python-bottle.html
# without cherrypy?
# requires ssl

# to create a server certificate, run eg
# openssl req -new -x509 -keyout server.pem -out server.pem -days 365 -nodes
# DON'T distribute this combined private/public key to clients!
# (see http://www.piware.de/2011/01/creating-an-https-server-in-python/#comment-11380)
import os

from datetime import datetime
import json

from bottle import Bottle, get, request, response, run, ServerAdapter
from bottle.ext import sqlalchemy
from sqlalchemy import create_engine, Column, DateTime, Integer, Sequence, String
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()
engine = create_engine(os.environ.get('DATABASE_URL', 'sqlite:///sqlite.db'), echo=True)

app = Bottle()
plugin = sqlalchemy.Plugin(
    engine,
    Base.metadata,
    keyword='db',
    create=True,
    commit=True,
    use_kwargs=False,
)

app.install(plugin)

class Measure(Base):
    __tablename__ = 'measures'
    id = Column(Integer, Sequence('id_seq'), primary_key=True)
    device = Column(String(32))
    timestamp = Column(DateTime(timezone=False))
    bssid = Column(String(32))
    ssid = Column(String(64))
    rssi = Column(Integer())

    def __init__(self, **kwargs):
        self.device = kwargs['device']
        self.timestamp = datetime.utcfromtimestamp(kwargs['ts'])
        self.bssid = kwargs['bssid']
        self.ssid = kwargs['ssid']
        self.rssi = kwargs['rssi']

    def __repr__(self):
        return "<Measure [{dev} @ {ts} for {bssid}] {r} dBm>".format(
            dev=self.device,
            ts=self.timestamp,
            bssid=self.bssid,
            r=self.rssi,
        )

    def to_json(self):
        return {
            'device': self.device,
            'ts': int(self.timestamp.timestamp()),
            'bssid': self.bssid,
            'ssid': self.ssid,
            'rssi': self.rssi,
        }



# copied from bottle. Only changes are to import ssl and wrap the socket
class SSLWSGIRefServer(ServerAdapter):
    def run(self, handler):
        from wsgiref.simple_server import make_server, WSGIRequestHandler
        import ssl
        if self.quiet:
            class QuietHandler(WSGIRequestHandler):
                def log_request(*args, **kw): pass
            self.options['handler_class'] = QuietHandler
        srv = make_server(self.host, self.port, handler, **self.options)
        srv.socket = ssl.wrap_socket (
         srv.socket,
         certfile='server.pem',  # path to certificate
         server_side=True)
        srv.serve_forever()

@app.get('/')
def ping():
    print("Got a ping")
    response.set_header('Content-Type', 'text/plain')
    return 'PONG\n'


def get_url(relative_url):
    (scheme, host, _, _, _) = request.urlparts
    return scheme + '://' + host + relative_url

@app.get('/data/<mac>')
def get_measures_for_mac(mac, db):
    from sqlalchemy.sql import select
    page = int(request.query.page or 0)
    per_page = 10
    measures = db.query(Measure).filter_by(device=mac).order_by(Measure.timestamp)[page * per_page:(page + 1) * per_page + 1]

    has_next = False
    if len(measures) > page:
        measures = [m.to_json() for m in list(measures)[:per_page]]
        has_next = True

    response.set_header('Content-Type', 'application/json')
    response.set_header('X-HATEOAS', '1')
    return json.dumps({
        'results': measures,
        'prev_page': get_url('/data/{mac}?page={p}'.format(mac=mac, p=page - 1)) if page > 0 else None,
        'next_page': get_url('/data/{mac}?page={p}'.format(mac=mac, p=page + 1)) if has_next else None
    })


@app.get('/data')
def get_macs(db):
    from sqlalchemy.sql import select
    print("Measures shas {}".format(dir(Measure)))
    devices = db.execute(select([Measure.__table__.c.device]).distinct()).fetchall()
    response.set_header('Content-Type', 'application/json')

    baseurl = get_url('/data/')

    return json.dumps([
        { 'device_id': d['device'],
          'url': baseurl + d['device'],
        }
        for d in devices
    ])

@app.post("/data/<mac>")
def post_data(mac, db):
    print("Retrieved measures for MAC: {}".format(mac))
    print("Request: {}".format(request))
    print("Values: {}".format(request.json))
    if request.json and len(request.json) > 0:
        for v in request.json:
            v['device'] = mac
            db.add(Measure(**v))
    else:
        print("Values were missing")
    response.set_header('Content-Type', 'text/plain')
    return 'CONN OK'

#instead of:
#run(host="0.0.0.0", port=8090)
#we use:

if __name__ == '__main__':
    if os.environ.get('SSL', None) in set(['TRUE', 'true', '1']):
        srv = SSLWSGIRefServer(host="0.0.0.0", port=4443)
        app.run(server=srv, reloader=True)
    else:
        app.run(host='0.0.0.0', port=4443, reloader=True)

