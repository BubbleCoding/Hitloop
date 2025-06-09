from app import create_app

# The application factory function is called to create an application instance
app = create_app()

if __name__ == '__main__':
    # The development server is run.
    # In production, a proper WSGI server like Gunicorn or uWSGI should be used.
    app.run(host='0.0.0.0', port=5000, debug=True) 