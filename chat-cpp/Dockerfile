FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make

WORKDIR /app

COPY . .

RUN g++ -o server server.cpp -pthread
RUN g++ -o client client.cpp -pthread

FROM ubuntu:22.04

WORKDIR /app

COPY --from=builder /app/server .
COPY --from=builder /app/client .

EXPOSE 1601

CMD ["./server"]