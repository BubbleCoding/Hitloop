let scanners = {};
const boxSize = 120;
const padding = 20;

// Selection state
let selectedScanners = new Set();
let selectionRect = { x: 0, y: 0, w: 0, h: 0 };
let isDragging = false;

function setup() {
    let canvasContainer = document.getElementById('canvas-container');
    let canvas = createCanvas(windowWidth * 0.9, windowHeight * 0.7);
    canvas.parent('canvas-container');
    textAlign(CENTER, CENTER);

    buildControlPanel();

    fetchScanners();
    // Refresh every 5 seconds to keep the view updated
    setInterval(fetchScanners, 5000);
}

function draw() {
    background(240);
    drawGrid();
    if (isDragging) {
        drawSelectionRect();
    }
}

function windowResized() {
    resizeCanvas(windowWidth * 0.9, windowHeight * 0.7);
}

// --- API Calls ---
async function fetchScanners() {
    try {
        const response = await fetch('/devices');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        scanners = await response.json();
    } catch (error) {
        console.error("Error fetching scanners:", error);
    }
}

async function configureScanners(scannerIds, config) {
    if (scannerIds.size === 0) {
        alert("No scanners selected!");
        return;
    }

    const requests = Array.from(scannerIds).map(id => {
        return fetch(`/configure/${id}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(config),
        });
    });

    try {
        const responses = await Promise.all(requests);
        responses.forEach(res => {
            if (!res.ok) console.error(`Failed to configure a scanner: ${res.status}`);
        });
        console.log(`Configuration sent to ${scannerIds.size} scanners.`);
        // Optionally clear selection after successful configuration
        selectedScanners.clear();
    } catch (error) {
        console.error("Error sending configurations:", error);
    }
}


// --- Drawing ---
function drawGrid() {
    const scannerNames = Object.keys(scanners);

    if (scannerNames.length === 0) {
        fill(0); noStroke(); textSize(16);
        text("No active scanners found in the last 5 minutes.", width / 2, height / 2);
        return;
    }

    const cols = Math.floor((width - padding) / (boxSize + padding));
    if (cols === 0) return;

    const startX = (width - cols * (boxSize + padding) + padding) / 2;
    
    scannerNames.forEach((scannerName, i) => {
        const col = i % cols;
        const row = Math.floor(i / cols);
        const x = startX + col * (boxSize + padding);
        const y = padding + row * (boxSize + padding);
        const scannerBox = { x, y, w: boxSize, h: boxSize, name: scannerName };
        
        drawScannerBox(scannerBox);
    });
}

function drawScannerBox(scannerBox) {
    const { x, y, w, h, name } = scannerBox;
    const scannerData = scanners[name];

    // Highlight if selected
    if (selectedScanners.has(name)) {
        stroke(0, 255, 0); // Green for selected
        strokeWeight(3);
    } else {
        stroke(50);
        strokeWeight(1);
    }

    fill(255);
    rect(x, y, w, h, 10);

    // Display Scanner Info
    noStroke();
    fill(0);
    
    textSize(14); textStyle(BOLD);
    text(name, x + w / 2, y + 15);
    
    textSize(11); textStyle(NORMAL);
    if (scannerData.movement && Object.keys(scannerData.movement).length > 0) {
        const mov = scannerData.movement;
        text(`XZ: ${mov.avgAngleXZ.toFixed(1)}`, x + w / 2, y + 40);
        text(`YZ: ${mov.avgAngleYZ.toFixed(1)}`, x + w / 2, y + 55);
        text(`Move: ${mov.totalMovement.toFixed(1)}`, x + w / 2, y + 70);
    } else {
        text("No movement data", x + w / 2, y + 55);
    }

    if (scannerData.timestamp) {
        textSize(10); textStyle(ITALIC);
        text(new Date(scannerData.timestamp).toLocaleTimeString(), x + w / 2, y + h - 15);
    }
}

function drawSelectionRect() {
    stroke(0, 100, 255, 150);
    fill(0, 100, 255, 50);
    rect(selectionRect.x, selectionRect.y, selectionRect.w, selectionRect.h);
}


// --- Mouse Events for Selection ---
function mousePressed() {
    if (mouseY > height) return; // Ignore clicks on the control panel
    isDragging = true;
    selectionRect.x = mouseX;
    selectionRect.y = mouseY;
    selectionRect.w = 0;
    selectionRect.h = 0;
    
    if (!keyIsDown(SHIFT)) {
        selectedScanners.clear();
    }
}

function mouseDragged() {
    if (!isDragging) return;
    selectionRect.w = mouseX - selectionRect.x;
    selectionRect.h = mouseY - selectionRect.y;
}

function mouseReleased() {
    if (!isDragging) return;
    isDragging = false;
    
    const selX = min(selectionRect.x, selectionRect.x + selectionRect.w);
    const selY = min(selectionRect.y, selectionRect.y + selectionRect.h);
    const selW = abs(selectionRect.w);
    const selH = abs(selectionRect.h);

    const scannerNames = Object.keys(scanners);
    const cols = Math.floor((width - padding) / (boxSize + padding));
    if (cols === 0) return;
    const startX = (width - cols * (boxSize + padding) + padding) / 2;

    scannerNames.forEach((scannerName, i) => {
        const col = i % cols;
        const row = Math.floor(i / cols);
        const x = startX + col * (boxSize + padding);
        const y = padding + row * (boxSize + padding);

        if (x + boxSize > selX && x < selX + selW && y + boxSize > selY && y < selY + selH) {
            selectedScanners.add(scannerName);
        }
    });
}

// --- Dynamic UI Builder ---
function buildControlPanel() {
    const panel = document.getElementById('control-panel');
    panel.innerHTML = '<h3>Behavior Controls (Shift-click to add to selection)</h3>';

    const behaviors = {
        "LED": [
            { name: "Off" },
            { name: "Solid", params: { color: 'color' } },
            { name: "Breathing", params: { color: 'color' } },
            { name: "HeartBeat", params: { color: 'color', period: 'number' } },
            { name: "Cycle", params: { color: 'color', delay: 'number' } },
        ],
        "Vibration": [
            { name: "Off" },
            { name: "Constant", params: { intensity: 'number' } },
            { name: "Burst", params: { intensity: 'number', frequency: 'number' } },
            { name: "Pulse", params: { intensity: 'number', frequency: 'number' } },
        ]
    };

    for (const category in behaviors) {
        const categoryDiv = document.createElement('div');
        categoryDiv.className = 'behavior-category';
        categoryDiv.innerHTML = `<h4>${category}</h4>`;

        behaviors[category].forEach(b => {
            const form = document.createElement('form');
            form.className = 'behavior-form';
            form.dataset.behaviorType = b.name;
            form.dataset.category = category.toLowerCase();
            
            let content = `<h5>${b.name}</h5>`;
            if (b.params) {
                for (const pName in b.params) {
                    content += `<label>${pName}: <input type="${b.params[pName]}" id="${b.name}-${pName}" value="${b.params[pName] === 'color' ? '#FFFF00' : '100'}"></label>`;
                }
            }
            content += '<button type="submit">Apply</button>';
            form.innerHTML = content;

            form.addEventListener('submit', (e) => {
                e.preventDefault();
                const behaviorConfig = { type: b.name };
                if (b.params) {
                    behaviorConfig.params = {};
                    for (const pName in b.params) {
                        const input = e.target.querySelector(`#${b.name}-${pName}`);
                        const value = b.params[pName] === 'number' ? parseFloat(input.value) : input.value;
                        behaviorConfig.params[pName] = value;
                    }
                }
                const payload = category === 'LED' ? { led_behavior: behaviorConfig } : { vibration_behavior: behaviorConfig };
                configureScanners(selectedScanners, payload);
            });
            categoryDiv.appendChild(form);
        });
        panel.appendChild(categoryDiv);
    }
} 