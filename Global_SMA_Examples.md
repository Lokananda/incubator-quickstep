# Global SMA Index


This readme gives brief implmentation details of Global SMA Index in Quickstep and examples of how to test it.


## Implmentation Details

### Creating a Global SMA

In our current implementation, we have modified the \analyze command to build a Global SMA Index for all the tables in the table which have SMA Index.

### Using Global SMA in SELECT queries

We have modified the Select Operator to use the Global SMA Index if it the relation has SMA configured (tables with no SMA will be unaffected by our code changes during execution)

## Examples scripts

1. To test Global SMA, build this codebase using default build procedure of quickstep

2. Create sample tables as in the script Global_SMA_examples/create_sql.sql

e.g.

`CREATE TABLE lineorder14 (
  lo_orderkey      INT NOT NULL,
  lo_linenumber    INT NOT NULL,
  lo_custkey       INT NOT NULL,
  lo_orderpriority CHAR(200) NOT NULL
) WITH BLOCKPROPERTIES (
TYPE split_rowstore,
BLOCKSIZEMB 2);`

CREATE INDEX sma_lo ON lineorder14 USING SMA;

3. Populate the table lineorder14 with the data

  * Run 8M_bulk_insert (obtained by compiling gen_wload_table_4col_bulk_insert.cpp) to create data with uniform distribution.

  * Bulk insert these tuples into the database


4. Building Global SMA Index for the above table

Run ./quickstep_cli

quickstep> \analyse


5. Execute sample select query to test the performance of Quickstep with Global SMA at 10% selectivity

`SELECT * FROM lineorder14_c WHERE lo_custkey<=499;`

Note 'lo_custkey' is a sorted column


6. Execute sample select query to test the performance of Quickstep with Global SMA at 1% selectivity

`SELECT * FROM lineorder14_c WHERE lo_custkey<=49;`

7. Observe that query response time for different select queries

## Major QuickStep Files Modified

cli/CommandExecutor.cpp

catalog/CatalogRelation.cpp

catalog/CatalogRelation.hpp

relational_operators/SelectOperator.cpp

relational_operators/SelectOperator.hpp

Other files and Folder

Global_SMA_workload directory has the script to generate data for our workload

Global_SMA_examples directory has SQL commands to execute the workload

