FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY ESP32FlaskServer.py .

EXPOSE 5000

ENV FLASK_APP=ESP32FlaskServer.py
CMD ["flask", "run", "--host=0.0.0.0", "--port=5000", "--debug"] 