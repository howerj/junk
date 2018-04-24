cangraduate(X) :- passed(X, senioryear).
passed(X, senioryear) :- passed(X, junioryear), goodgrades(X,senior).
passed(X, junioryear) :- passed(X, sophomoreyear), goodgrades(X,junior).
passed(X, sophomoreyear) :- passed(X, freshmanyear), goodgrades(X,sophomore).
passed(X, freshmanyear) :- showeduptoclass(X).

showeduptoclass(mary).
goodgrades(mary,sophomore).
goodgrades(mary,junior).
goodgrades(mary,senior).

nevershoweduptoclass(john).
goodgrades(john,sophomore).
goodgrades(john,junior).
goodgrades(john,senior).

?- cangraduate(X).
