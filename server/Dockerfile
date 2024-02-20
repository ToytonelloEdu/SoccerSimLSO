FROM alpine:latest
RUN mkdir -p /app
COPY server /app
WORKDIR /app
RUN chmod +x server
CMD ["/app/server"] 
