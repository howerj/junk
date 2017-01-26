unhealthy(X) :- overweight(X), noexercise(X).
overweight(X) :- bmi(X,high).

bmi(john,high).
noexercise(john).

bmi(mary,low).
noexercise(mary).

?- unhealthy(john).
?- unhealthy(mary).
