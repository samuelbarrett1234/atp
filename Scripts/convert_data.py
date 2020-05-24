import sqlite3 as sql
import pandas as pd

db = sql.connect("../Data/DB/eqlogic.db")

# this script is for converting all theorem data for a given context to a
# CSV file

ctx = "group-theory"

df = pd.read_sql_query("SELECT thm_id, stmt, "
    "(CASE WHEN proof IS NULL THEN 0 ELSE 1 END) AS is_proven, "
    "time_agg, max_mem_agg, num_exp_agg, num_attempts "
    "FROM theorems LEFT OUTER NATURAL JOIN proofs NATURAL JOIN "
    "(SELECT thm_id, SUM(time_cost) AS time_agg, MAX(max_mem) AS max_mem_agg, "
	"SUM(num_expansions) AS num_exp_agg, COUNT(thm_id) AS num_attempts "
	"FROM proof_attempts GROUP BY thm_id) NATURAL JOIN model_contexts "
    "WHERE name = '" + ctx + "'", db)

db.close()

df.to_csv("../Data/DB/thms.csv", index=False)

