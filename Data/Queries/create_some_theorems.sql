
/* 1 should be the ID of "group theory" */
INSERT INTO theorems (stmt, ctx) VALUES ("i(i(x0)) = x0", 1);
INSERT INTO theorems (stmt, ctx) VALUES ("i(*(x0, x1)) = *(i(x1), i(x0))", 1);
INSERT INTO theorems (stmt, ctx) VALUES ("e = *(*(x0, x1), i(*(x0, x1)))", 1);
