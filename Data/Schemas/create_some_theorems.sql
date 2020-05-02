
/* 1 should be the ID of "group theory" */
INSERT INTO theorems (stmt, ctx) VALUES ("i(i(x0)) = x0", 1);
INSERT INTO theorems (stmt, ctx) VALUES ("i(*(x, y)) = *(i(y), i(x))", 1);
INSERT INTO theorems (stmt, ctx) VALUES ("e = *(*(x, y), i(*(x, y)))", 1);
