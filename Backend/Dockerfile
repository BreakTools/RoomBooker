FROM python:3.11-slim

WORKDIR /RoomBooker/Backend

COPY . .

RUN pip install --no-cache-dir -r requirements.txt

CMD ["python", "./main.py"]