README EM  CONSTRUCAO 


build docker 
docker build -t dev_bank_with_drogon_framework:latest . 

docker run -p5432:5432 --name pg_database --network docker_local_network  -e POSTGRES_PASSWORD=123456 -d postgres
docker run -p5432:5432 --name pg_database --network docker_local_network  -e POSTGRES_PASSWORD=123456 -d postgres
docker-compose down --volumes
docker-compose up  