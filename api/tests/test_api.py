import time
from http import HTTPStatus
from io import BytesIO
from unittest import TestCase, mock

from app import gatekeeper
from PIL import Image
from pyotp import TOTP


class TestApi(TestCase):

    def fakeCapture(self):
        capture = Image.open('tests/fixtures/gatePhoto.jpeg')
        imgByteArr = BytesIO()
        capture.save(imgByteArr, format='JPEG')
        return imgByteArr

    def setUp(self):
        self.api = gatekeeper.create_api()
        self.client = self.api.test_client()

    def test_404_at_root(self):
        """Test that a request to / returns a 404."""
        resp = self.client.get('/')
        self.assertEqual(HTTPStatus.NOT_FOUND, resp.status_code)

# Toggle (open/close) gate
    def test_401_at_toggle_gate_without_totp_token(self):
        """Test that a request to /toggle returns a 401."""
        resp = self.client.post('/toggle')
        self.assertEqual(HTTPStatus.UNAUTHORIZED, resp.status_code)

    def test_201_at_toggle_gate_with_totp_token(self):
        """Test that a request to /toggle returns a 201."""
        totp = TOTP('VALID')
        headers = { 'Authorization': totp.now() }  
        
        resp = self.client.post('/toggle', headers=headers)
        self.assertEqual(HTTPStatus.ACCEPTED, resp.status_code)

    def test_401_at_toggle_gate_with_totp_token(self):
        """Test that a request to /toggle returns a 401."""
        totp = TOTP('INVALID')
        headers = { 'Authorization': totp.now() }  

        resp = self.client.post('/toggle', headers=headers)
        self.assertEqual(HTTPStatus.UNAUTHORIZED, resp.status_code)

# Capture, return photo with current gate state
    def test_content_is_image_at_capture(self):
        """Test that a request to /capture returns a 200."""

        resp = self.client.get('/capture')
        self.assertEqual(HTTPStatus.OK, resp.status_code)
        self.assertEqual('image/jpeg', resp.content_type)
        self.assertEqual(self.fakeCapture().read(), resp.data)
    
    @mock.patch('app.cameraClient')
    def test_500_at_capture_when_image_failed(self, cameraMock):
        """Test that a request to /capture returns a 200."""
        self.client.get('/capture')
        cameraMock.capture.assert_called_once()