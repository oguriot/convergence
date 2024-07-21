./configure
gmake
su
gmake install
adduser postgres
mkdir /usr/local/pgsql/data
chown postgres /usr/local/pgsql/data
su - postgres
/usr/local/pgsql/bin/initdb -D /usr/local/pgsql/data
/usr/local/pgsql/bin/postgres -D /usr/local/pgsql/data >logfile 2>&1 &
--#########################################################
postgres@kremlin:~$ psql
postgres=# CREATE ROLE userconfluenti WITH CREATEDB LOGIN PASSWORD 'passconfluenti';
postgres=# CREATE DATABASE baseconfluenti WITH OWNER = userconfluenti;
postgres=# ALTER ROLE userconfluenti WITH LOGIN;
--#########################################################
postgres@kremlin:~$ psql -U userconfluenti baseconfluenti -h localhost
create table tableconfluenti();

alter table tableconfluenti add column "conf_id"  varchar(80);
alter table tableconfluenti add column "name"  varchar(80);
alter table tableconfluenti add column "first_name"  varchar(80);
alter table tableconfluenti add column "region"  varchar(80);
alter table tableconfluenti add column "arrondis"  varchar(80);
alter table tableconfluenti add column "quartier"  varchar(80);
alter table tableconfluenti add column "address"  varchar(80);
alter table tableconfluenti add column "date_of_birth"  varchar(80);
alter table tableconfluenti add column "place_of_birth"  varchar(80);
alter table tableconfluenti add column "height"  varchar(5);
alter table tableconfluenti add column "weight"  varchar(5);
alter table tableconfluenti add column "hair_color"  varchar(80);
alter table tableconfluenti add column "eyes_color"  varchar(80);
alter table tableconfluenti add column "sex"  varchar(80);
alter table tableconfluenti add column "citizenship"  varchar(80);
alter table tableconfluenti add column "language"  varchar(80);
alter table tableconfluenti add column "mark"  varchar(100);
alter table tableconfluenti add column "status"  varchar(200);
alter table tableconfluenti add column "current_desc"  varchar(500);

ALTER ROLE userconfluenti WITH Superuser;

SELECT * FROM tableconfluenti;
