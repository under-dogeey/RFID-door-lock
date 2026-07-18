FROM python:3.14.3

WORKDIR /app

COPY requirements.txt .

RUN pip3 install -r requirements.txt

COPY server.py .

EXPOSE 8000

ENTRYPOINT ["python", "server.py"]
