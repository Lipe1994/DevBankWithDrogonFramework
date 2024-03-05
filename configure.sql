SHOW MAX_CONNECTIONS;

ALTER SYSTEM SET max_connections = 230;

CREATE TABLE public.customers (
	id smallint NOT NULL,
	balance int NOT NULL,
	"limit" int NOT NULL,
	"version" int NOT NULL,
	"created_at" timestamptz NOT NULL,
	"update_at" timestamptz NULL,
	CONSTRAINT "uq_customers_id" PRIMARY KEY (id)
);

CREATE TABLE public.transactions (
	id int NOT NULL GENERATED BY DEFAULT AS IDENTITY( INCREMENT BY 1 MINVALUE 1 MAXVALUE 10000000 START 1 CACHE 1 NO CYCLE),
	"amount" int NOT NULL,
	"description" varchar(10) NOT NULL,
	"type" bpchar(1) NOT NULL,
	"created_at" timestamptz NOT NULL,
	"customer_id" smallint NOT NULL,
	CONSTRAINT fk_transactions_customer_id FOREIGN KEY (customer_id) REFERENCES public.customers(id),
	CONSTRAINT "uq_transactions_id" PRIMARY KEY (id)
);


INSERT INTO public.customers (id, balance, "limit", "version", "created_at", "update_at")
VALUES (1, 0, 100000, 0, CURRENT_TIMESTAMP, NULL);

INSERT INTO public.customers (id, balance, "limit", "version", "created_at", "update_at")
VALUES (2, 0, 80000, 0, CURRENT_TIMESTAMP, NULL);

INSERT INTO public.customers (id, balance, "limit", "version", "created_at", "update_at")
VALUES (3, 0, 1000000, 0, CURRENT_TIMESTAMP, NULL);

INSERT INTO public.customers (id, balance, "limit", "version", "created_at", "update_at")
VALUES (4, 0, 10000000, 0, CURRENT_TIMESTAMP, NULL);

INSERT INTO public.customers (id, balance, "limit", "version", "created_at", "update_at")
VALUES (5, 0, 500000, 0, CURRENT_TIMESTAMP, NULL);


-- Functions 
CREATE OR REPLACE FUNCTION creditar(
	customerId smallint, 
	amount INT, 
	description varchar(10)
)
	RETURNS TABLE(
	    affectedRow BOOLEAN, 
	    "_limit" INT, 
	    _balance INT
	) 
	LANGUAGE plpgsql AS 
$func$
DECLARE
    updated_limit INT;
    updated_balance INT;
BEGIN

    UPDATE public.customers SET balance = balance + amount WHERE id = customerId 
		RETURNING "limit", balance INTO updated_limit, updated_balance;
  
    IF FOUND THEN
    	INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES (customerId, amount, description, 'c', NOW());
        RETURN QUERY SELECT true, updated_limit, updated_balance;
    ELSE
        RETURN QUERY SELECT false, 0, 0;
    END IF;
END;
$func$;

CREATE OR REPLACE FUNCTION debitar(
	customerId smallint, 
	amount INT, 
	description varchar(10)
)
	RETURNS TABLE(
	    affectedRow BOOLEAN, 
	    "_limit" INT, 
	    _balance INT
	) 
	LANGUAGE plpgsql AS 
$func$
DECLARE
    updated_limit INT;
    updated_balance INT;
BEGIN

    update public.customers SET balance = balance - @amount WHERE id = @customerId AND ((balance - @amount) >= -"limit")
		RETURNING "limit", balance INTO updated_limit, updated_balance;
  
    IF FOUND THEN
    	INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES (customerId, amount, description, 'd', NOW());
        RETURN QUERY SELECT true, updated_limit, updated_balance;
    ELSE
        RETURN QUERY SELECT false, 0, 0;
    END IF;
END;
$func$;