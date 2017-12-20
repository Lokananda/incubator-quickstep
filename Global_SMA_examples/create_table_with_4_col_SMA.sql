CREATE TABLE lineorder14 (
  lo_orderkey      INT NOT NULL,
  lo_linenumber    INT NOT NULL,
  lo_custkey       INT NOT NULL,
  lo_orderpriority CHAR(200) NOT NULL
) WITH BLOCKPROPERTIES (
TYPE split_rowstore,
BLOCKSIZEMB 2);

CREATE INDEX sma_lo ON lineorder14 USING SMA;
