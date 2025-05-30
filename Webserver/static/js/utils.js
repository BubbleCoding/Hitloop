// Utility functions for the simulation

// Simulates RSSI from distance.
// Depends on p5.js global 'map' function and cfg object (from config.js)
function distanceToRSSI(distance) {   
    const maxDist = Math.sqrt(cfg.canvasWidth * cfg.canvasWidth + cfg.canvasHeight * cfg.canvasHeight);
    if (distance < 1) distance = 1; 
    let effectiveDistance = Math.min(distance, maxDist / 2); 
    let rssi = map(effectiveDistance, 0, maxDist / 2, cfg.rssiAtMinDist, cfg.rssiAtEffectiveMaxDist);
    return Math.round(rssi);
} 