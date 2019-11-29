from app import gatekeeper
from unittest import TestCase
from http import HTTPStatus
from pyotp import TOTP
import time

class TestApi(TestCase):
    def setUp(self):
        self.api = gatekeeper.create_api()
        self.client = self.api.test_client()

    def test_404_at_root(self):
        """Test that a request to / returns a 404."""
        resp = self.client.get('/')
        self.assertEqual(HTTPStatus.NOT_FOUND, resp.status_code)

    def test_401_at_toggle_gate_without_totp_token(self):
        """Test that a request to / returns a 401."""
        resp = self.client.post('/toggle')
        self.assertEqual(HTTPStatus.UNAUTHORIZED, resp.status_code)

    def test_201_at_toggle_gate_with_totp_token(self):
        """Test that a request to / returns a 201."""
        totp = TOTP('VALID')
        headers = { 'Authorization': totp.now() }  
        
        resp = self.client.post('/toggle', headers=headers)
        self.assertEqual(HTTPStatus.ACCEPTED, resp.status_code)

    def test_401_at_toggle_gate_with_totp_token(self):
        """Test that a request to / returns a 401."""
        totp = TOTP('INVALID')
        headers = { 'Authorization': totp.now() }  

        resp = self.client.post('/toggle', headers=headers)
        self.assertEqual(HTTPStatus.UNAUTHORIZED, resp.status_code)