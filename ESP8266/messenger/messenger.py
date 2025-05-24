#!/usr/bin/env python3

#Version: 0.1
#Author: Remko Kleinjan
#Date: 231122

import cherrypy, socket, json, urllib.parse, logging
from logging import handlers

mdb = {}

class Messenger():
  """
  ESP8266 Messenger
  """

  # === init ===
  def __init__(self):
    """
    init
    """
    cherrypy.log('Starting...')

class MessengerAPI():
  exposed = True

  @cherrypy.tools.json_out()
  def send(self, v, user):
    """
        route='/{v}/send/{user}'
        conditions={'method': ['GET']}
    """
    if v == 'v1':
      data = urllib.parse.unquote(user)
      if data:
        MessengerDB().add_memdb(data)
        return {'result': {'send': 'OK'}}
      else:
        raise cherrypy.HTTPError(400, 'Unknown user')
    else:
      raise cherrypy.HTTPError(400, 'Unknown version')

  @cherrypy.tools.json_out()
  def get(self, v, user):
    """
        route='/{v}/get/{user}'
        conditions={'method': ['GET']}
    """
    if v == 'v1':
      data = urllib.parse.unquote(user)
      if data:
        result = MessengerDB().get_memdb(data)
        return {'result': {'messages': result}}
      else:
        raise cherrypy.HTTPError(400, 'Unknown user')
    else:
      raise cherrypy.HTTPError(400, 'Unknown version')

  @cherrypy.tools.json_out()
  def read(self, v, user):
    """
        route='/{v}/read/{user}'
        conditions={'method': ['GET']}
    """
    if v == 'v1':
      data = urllib.parse.unquote(user)
      if data:
        MessengerDB().remove_memdb(data)
        return {'result': {'read': 'OK'}}
      else:
        raise cherrypy.HTTPError(400, 'Unknown user')
    else:
      raise cherrypy.HTTPError(400, 'Unknown version')

class MessengerDB():
  """
  DB
  """
  def add_memdb(self, name):
    global mdb
    user = {}

    if name in mdb.keys():
      if int(mdb[name]['messages']) < 1000:
        user['messages'] = mdb[name]['messages'] + 1
    else:
      user['messages'] = 1
    mdb[name] = user

  def get_memdb(self, name):
    if name in mdb.keys():
      count = mdb[name]['messages']
    else:
      raise cherrypy.HTTPError(400, 'User not found')

    return count

  def remove_memdb(self, name):
    global mdb

    if name in mdb.keys():
      if int(mdb[name]['messages']) > 0:
        mdb[name]['messages'] = mdb[name]['messages'] - 1
    else:
      raise cherrypy.HTTPError(400, 'User not found')

# === JSON Error ===
def json_error(status, message, traceback, version):
  """
  JSONify all CherryPy error responses (created by raising the cherrypy.HTTPError exception)
  """
  cherrypy.log('Status {0}: {1}'.format(status, message))

  cherrypy.response.headers['Content-Type'] = 'application/json'
  response_body = json.dumps({
    'error': {
      'http_status': status,
      'message': message,
    }
  })

  cherrypy.response.status = status
  return response_body

# === Logs ===
def custom_logs():
  """
  Custom log handler for rotating logs.
  """
  log = cherrypy.log

  file_name = 'access.log'
  h = handlers.TimedRotatingFileHandler(file_name, 'midnight', 1, 7)
  h.setLevel(logging.DEBUG)
  h.setFormatter(cherrypy._cplogging.logfmt)
  log.access_log.addHandler(h)
  log.access_log.propagate = True

  file_name = 'messenger.log'
  h = handlers.TimedRotatingFileHandler(file_name, 'midnight', 1, 7)
  h.setLevel(logging.DEBUG)
  h.setFormatter(cherrypy._cplogging.logfmt)
  log.error_log.addHandler(h)
  log.error_log.propagate = True

if __name__ == '__main__':
  """
  URL Routing.
  """

  dispatcher = cherrypy.dispatch.RoutesDispatcher()

  # send
  dispatcher.connect(name='send',
                      route='/{v}/send/{user}',
                      controller=MessengerAPI(),
                      action='send',
                      conditions={'method': ['GET']})
  # get
  dispatcher.connect(name='get',
                      route='/{v}/get/{user}',
                      controller=MessengerAPI(),
                      action='get',
                      conditions={'method': ['GET']})
  # read
  dispatcher.connect(name='read',
                      route='/{v}/read/{user}',
                      controller=MessengerAPI(),
                      action='read',
                      conditions={'method': ['GET']})

  """
  CherryPy configuration.
  """
  conf = {
    'global': {
        'server.socket_host': socket.getfqdn(),
        'server.socket_port': 2512,
        'server.thread_pool': 32,
#        'server.ssl_module': 'builtin',
#        'server.ssl_certificate': '/etc/ssl/ssl.crt',
#        'server.ssl_private_key': '/etc/ssl/ssl.key',
        'request.show_tracebacks': True,
        'log.screen': True,
    },
    '/': {
      'request.dispatch': dispatcher,
      'error_page.default': json_error,
#      'tools.auth_basic.on': True,
#      'tools.auth_basic.realm': 'localhost',
#      'tools.auth_basic.checkpassword': auth,
    }
  }

  #cherrypy.process.plugins.Monitor(cherrypy.engine, p.tasks, frequency=1, name='Tasks').subscribe()

  cherrypy.process.plugins.Daemonizer(cherrypy.engine).subscribe()
  cherrypy.engine.autoreload.unsubscribe()
  cherrypy.process.plugins.DropPrivileges(cherrypy.engine, uid='www-data', gid='www-data').subscribe()
  custom_logs()
  cherrypy.quickstart(Messenger(), '/', conf)
