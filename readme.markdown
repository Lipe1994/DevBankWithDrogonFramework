### Submissão para Rinha de Backend, Segunda Edição: 2024/Q1 - Controle de Concorrência
![Imagem que representa o Drogon](./drogon_img.jpeg)

##### Stack:
    - Drogon framework
    - Postgres
    - Nginx

##### Repositório
- [Lipe1994/DevBankWithDrogonFramework](https://github.com/Lipe1994/DevBankWithDrogonFramework/)

##### Filipe Ferreira:

 - [@filipe-ferreira-425380123](https://www.linkedin.com/in/filipe-ferreira-425380123/) - Linkedin

 - [@l1peferreira](https://www.instagram.com/l1peferreira/) - Instagram



###### Detalhes de execução e montagem do projeto:

 - Se não existir o diretório build, será necessário criá-lo
```shell
mkdir build
```

 - Compilar
```shell
cmake .. && make
```

 - Executar o projeto
```shell
./DevBankWithDrogonFramework
```

 - Dockerizar o projeto(O Dockerfile já está configurado para compilar dentro do container durante o build da imagem):
```shell
docker build -t lipeferreira1609/dev_bank_with_drogon_framework:latest .
```

 - Montar um volume com docker-compose:
```shell
docker-compose up -d
```

Desmontar um volume com docker-compose:
```shell
docker-compose down
```
