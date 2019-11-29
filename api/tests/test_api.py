from app import create_api
from unittest import TestCase
from http import HTTPStatus

class TestApi(TestCase):
    def setUp(self):
        self.api = create_api()
        self.client = self.api.test_client()

    def test_404_at_root(self):
        """Test that a request to / returns a 404."""
        resp = self.client.get('/')

        self.assertEqual(HTTPStatus.NOT_FOUND, resp.status_code)
    def test_200_at_toggle_gate(self):
        """Test that a request to / returns a 200."""
        resp = self.client.post('/toggle')

        self.assertEqual(HTTPStatus.ACCEPTED, resp.status_code)