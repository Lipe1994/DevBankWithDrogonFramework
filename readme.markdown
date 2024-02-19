# DevBak construído com Drogon Framework 

### Este projeto usa C++17
![Imagem que representa o Drogon](./drogon_img.jpeg)

Se não existir o diretório build, será necessário criá-lo
```shell
mkdir build
```

Compilar
```shell
cmake .. && make
```

Executar o projeto
```shell
./DevBankWithDrogonFramework
```

Dockerizar o projeto(O Dockerfile já está configurado para compilar dentro do container durante o build da imagem):
```shell
docker build -t lipeferreira1609/dev_bank_with_drogon_framework:latest .
```

Montar um volume com docker-compose:
```shell
docker-compose up -d
```

Desmontar um volume com docker-compose:
```shell
docker-compose down
```
