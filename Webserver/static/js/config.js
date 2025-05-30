const cfg = {
    numScanners: 10, // Initial number of scanners
    minScanners: 1,
    maxScanners: 50,
    scannerStep: 1,

    dataSendIntervalMs: 5000,

    canvasWidth: 800,
    canvasHeight: 600,
    backgroundColor: 230,

    beaconRadius: 10,
    beaconColor: [255, 0, 0, 150],
    beaconMargin: 30,
    beaconDistanceCircleStrokeColor: [100, 100, 100, 70],
    beaconDistanceCircleStrokeWeight: 1,

    scannerSize: 15,
    scannerColor: [0, 0, 255, 150],
    scannerInitialVelMagnitude: 1,
    
    scannerMaxSpeed: 0.1, // Initial max speed
    minScannerMaxSpeed: 0.1,
    maxScannerMaxSpeed: 5,
    scannerMaxSpeedStep: 0.1,

    scannerVelChangeMagnitude: 0.05, // Initial vel change magnitude
    minScannerVelChangeMag: 0.01,
    maxScannerVelChangeMag: 0.5,
    scannerVelChangeMagStep: 0.01,

    scannerRssiDisplayTextColor: [0, 0, 0],
    scannerRssiDisplayTextSize: 10,
    scannerRssiDisplayLineHeight: 12,

    rssiAtMinDist: -30,
    rssiAtEffectiveMaxDist: -90,
}; 