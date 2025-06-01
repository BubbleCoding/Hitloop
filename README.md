# Hitloop Beacon controller

> This repo is used for a hitloop project.
> Most folders are just testing stuff and should be removed.
> The ThingSpeak_BLE_scan file looks for BLE devices and sends the data to thingspeak.
> The iBeacons file is used for the eggs that send BLE information.

## Setup

### Docker

This project uses Docker and Docker Compose to manage the web application and documentation services.

**Services:**

* **`docs`**: Serves the project documentation using MkDocs.
  * Builds from `Dockerfile` in the project root.
  * Accessible at `http://localhost:8000`.
  * Live reloads on changes to documentation files.
* **`webserver`**: Runs the main web application.
  * Builds from `Webserver/Dockerfile`.
  * Accessible at `http://localhost:5000`.
  * Live reloads on changes to web application files in the `Webserver` directory.

**Commands:**

* **Build and run all services:**
  This command builds and starts all services. Ports are mapped to localhost as detailed in the 'Services' section (e.g., `docs` at `http://localhost:8000`, `webserver` at `http://localhost:5000`).

    ```bash
    docker-compose up --build
    ```

* **Build and run only the documentation service:**
  This command starts the `docs` service, accessible at `http://localhost:8000`.

    ```bash
    docker-compose up --build docs
    ```

* **Build and run only the webserver service:**
  This command starts the `webserver` service, accessible at `http://localhost:5000`.

    ```bash
    docker-compose up --build webserver
    ```

* **Stop and remove containers:**

    ```bash
    docker-compose down
    ```

    (Or press `Ctrl+C` in the terminal where `docker-compose up` is running)
