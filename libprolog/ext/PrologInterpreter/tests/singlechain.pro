likes(X, italian) :- likes(X,pizza),
                     likes(X,spaghetti),
                     likes(X,meatballs),
                     likes(X,ravioli),
                     likes(X,alfredo),
                     likes(X,pasta).

likes(mary,pizza).
likes(mary,spaghetti).
likes(mary,ravioli).
likes(mary,alfredo).
likes(mary,pasta).

likes(john,pizza).
likes(john,spaghetti).
likes(john,meatballs).
likes(john,ravioli).
likes(john,alfredo).
likes(john,pasta).

?- likes(X, italian).
