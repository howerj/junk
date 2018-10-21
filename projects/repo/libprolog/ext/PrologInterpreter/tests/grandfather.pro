parent(george,peter).
parent(george,samuel).
parent(samuel,albert).
parent(samuel,mike).

grandfather(B,N) :- parent(B,P), parent(P,N).
?- parent(george,peter).
?- grandfather(george,mike).
?- grandfather(X,albert).
?- grandfather(X,peter).
