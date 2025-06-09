let scanners = [];
const boxSize = 100;
const padding = 20;

function setup() {
    let canvasContainer = document.getElementById('canvas-container');
    let canvas = createCanvas(windowWidth * 0.9, windowHeight * 0.8);
    canvas.parent('canvas-container');
    textAlign(CENTER, CENTER);
    textSize(14);
    
    fetchScanners();
    // Refresh every 10 seconds
    setInterval(fetchScanners, 10000);
}

function draw() {
    background(240); // Light grey background
    drawGrid();
}

function windowResized() {
    resizeCanvas(windowWidth * 0.9, windowHeight * 0.8);
}

async function fetchScanners() {
    try {
        const response = await fetch('/devices');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        scanners = data;
    } catch (error) {
        console.error("Error fetching scanners:", error);
    }
}

function drawGrid() {
    if (scanners.length === 0) {
        fill(0);
        noStroke();
        text("No active scanners found in the last 5 minutes.", width / 2, height / 2);
        return;
    }

    const cols = Math.floor((width - padding) / (boxSize + padding));
    if (cols === 0) return; // Avoid division by zero if canvas is too small

    const startX = (width - cols * (boxSize + padding) + padding) / 2;
    
    for (let i = 0; i < scanners.length; i++) {
        const scanner = scanners[i];
        const col = i % cols;
        const row = Math.floor(i / cols);

        const x = startX + col * (boxSize + padding);
        const y = padding + row * (boxSize + padding);

        // Draw the scanner box
        stroke(50);
        fill(255);
        rect(x, y, boxSize, boxSize, 10); // Rounded corners

        // Draw the scanner name
        fill(0);
        noStroke();
        text(scanner.name, x, y, boxSize, boxSize);
    }
} 