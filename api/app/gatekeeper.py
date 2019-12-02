import time
from http import HTTPStatus
from io import BytesIO

from flask import Flask, Response, abort, request, send_file
from PIL import Image
from pyotp import TOTP

from . import api, cameraClient

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


@api.route('/capture', methods=['GET'])
def capture():
    try:
        capture = Image.open('tests/fixtures/gatePhoto.jpeg')
        img_io = BytesIO()
        capture.save(img_io, 'JPEG')
        cameraClient.capture()
        return send_file(img_io, mimetype='image/jpeg')

    except Exception as e:
        print(e)
        abort()

    
