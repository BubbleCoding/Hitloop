from datetime import datetime
from . import db

class Scanner(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    rssi_values = db.relationship('RssiValue', backref='scanner', lazy=True)
    movements = db.relationship('ScannerMovement', backref='scanner', lazy=True)

class Beacon(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80), unique=True, nullable=False)
    rssi_values = db.relationship('RssiValue', backref='beacon', lazy=True)

class RssiValue(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    rssi = db.Column(db.Integer, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    scanner_id = db.Column(db.Integer, db.ForeignKey('scanner.id'), nullable=False)
    beacon_id = db.Column(db.Integer, db.ForeignKey('beacon.id'), nullable=False)

class ScannerMovement(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    avg_angle_xz = db.Column(db.Float, nullable=False)
    avg_angle_yz = db.Column(db.Float, nullable=False)
    total_movement = db.Column(db.Float, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    scanner_id = db.Column(db.Integer, db.ForeignKey('scanner.id'), nullable=False) 