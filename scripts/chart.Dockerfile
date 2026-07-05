# Image for regenerating progress.png / progress.svg.
FROM python:3-slim

RUN pip install --no-cache-dir matplotlib

WORKDIR /app
