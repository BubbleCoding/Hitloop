FROM python:3.9-slim

WORKDIR /app

# Copy requirements first to leverage Docker cache
COPY Webserver/requirements.txt Webserver/requirements.txt
RUN pip install -r Webserver/requirements.txt

# Copy the rest of the application
COPY Webserver/ /app/Webserver/

# Set the working directory to the webserver root
WORKDIR /app/Webserver

EXPOSE 5000

# Run the application using the new entry point
CMD ["flask", "--app", "run", "run", "--host=0.0.0.0"] 