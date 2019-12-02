from app import gatekeeper, CameraClient

if __name__ == '__main__':
    app = gatekeeper.create_api(CameraClient.CameraClient('192.168.101.111'))
    app.run('0.0.0.0', 8000, debug=True)
