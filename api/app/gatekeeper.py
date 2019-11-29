from http import HTTPStatus
from pyotp import TOTP
import time

from flask import abort, Flask, request
from . import api

def create_api():
    app = Flask(__name__)
    app.register_blueprint(api)

    return app

@api.route('/toggle', methods=['POST'])
def toggle():
    totp = TOTP('VALID')
    token = request.headers.get('Authorization')

    if not token or not totp.verify(token):
        abort(HTTPStatus.UNAUTHORIZED)

    return ('', HTTPStatus.ACCEPTED)
