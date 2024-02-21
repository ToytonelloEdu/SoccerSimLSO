FROM alpine:latest
RUN apk add libc6-compat
WORKDIR /app
COPY . /app
CMD ["./server"] 

EXPOSE 8080