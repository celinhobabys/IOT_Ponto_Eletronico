# Start from the official Node-RED image
FROM nodered/node-red:latest

# Switch to root to install system dependencies
USER root

# Install PostgreSQL client libraries (required for node-postgres)
RUN apk add --no-cache postgresql-client

# Switch back to node-red user
USER node-red

# Install PostgreSQL node package (using the working one)
RUN npm install node-red-contrib-postgresql
