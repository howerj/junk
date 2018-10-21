likes(mary,wine).
startsdrinking(mary,atnight).

likes(john,liquor).
startsdrinking(john,beforenoon).

alcohol(wine).
alcohol(beer).
alcohol(liquor).

alcoholic(X) :- likes(X,Y), alcohol(Y), startsdrinking(X,beforenoon).

?- alcoholic(mary).
?- alcoholic(john).
