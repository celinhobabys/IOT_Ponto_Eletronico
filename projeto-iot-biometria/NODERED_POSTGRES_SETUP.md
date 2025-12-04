# Node-RED with PostgreSQL - Docker Setup

## ✅ Fixed and Working!

Your Node-RED and PostgreSQL containers are now running successfully.

## What Was Fixed

1. **Removed obsolete `version` field** from docker-compose.yml
2. **Installed PostgreSQL client libraries** in the container (required for node-postgres)
3. **Installed the correct PostgreSQL node package**: `node-red-contrib-postgresql`

## Container Status

- **Node-RED**: Running at http://localhost:1880
- **PostgreSQL**: Running internally on port 5432

## PostgreSQL Connection Details

When configuring PostgreSQL nodes in Node-RED, use these settings:

- **Host**: `postgres` (the service name in docker-compose)
- **Port**: `5432`
- **Database**: `nodered_db`
- **User**: `nodered`
- **Password**: `YOUR_SECURE_PASSWORD` (change this in docker-compose.yml!)

## Available PostgreSQL Nodes

The container now has `node-red-contrib-postgresql` installed, which provides:
- **postgresql** - Execute PostgreSQL queries
- **postgreSQLConfig** - Configure PostgreSQL connections

## Useful Commands

```bash
# Start containers
docker-compose up -d

# Stop containers
docker-compose down

# View Node-RED logs
docker logs nodered_app

# View PostgreSQL logs
docker logs nodered_postgres

# Rebuild after changes
docker-compose up --build -d

# Access Node-RED shell
docker exec -it nodered_app /bin/bash

# Access PostgreSQL shell
docker exec -it nodered_postgres psql -U nodered -d nodered_db
```

## Testing the Connection

1. Open Node-RED at http://localhost:1880
2. Drag a **postgresql** node from the palette
3. Configure it with the connection details above
4. Test with a simple query: `SELECT version();`

## ⚠️ Important Security Note

**Change the PostgreSQL password** in `docker-compose.yml` before using in production:

```yaml
POSTGRES_PASSWORD: "YOUR_SECURE_PASSWORD"
```

## Troubleshooting

If you see warnings about missing `node-red-contrib-postgres`:
- This is normal if you have old flow files
- The warning can be ignored
- Just use the `postgresql` nodes from `node-red-contrib-postgresql` package
- Old flow configurations will be cleaned up automatically

## Data Persistence

Your data is persisted in Docker volumes:
- `nodered_data`: Node-RED flows and settings
- `postgres_data`: PostgreSQL database files

These volumes persist even when containers are stopped or removed.
