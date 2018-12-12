sum_list([], 0).
sum_list([H | Rest], Sum) :- sum_list(Rest,Tmp), Sum is H + Tmp.

add_category_min_score(In, Category, Min,  P) :-
    findall(X, gerrit:commit_label(label(Category,X),R),Z),
    sum_list(Z, Sum),
    Sum >= Min, !,
    gerrit:commit_label(label(Category, V), U),
    V >= 1,
    !,
    P = [label(Category,ok(U)) | In].

add_category_min_score(In, Category,Min,P) :-
    P = [label(Category,need(Min)) | In].

submit_rule(S) :-
    gerrit:default_submit(X),
    X =.. [submit | Ls],
    gerrit:remove_label(Ls,label('Code-Review',_),NoCR),
    add_category_min_score(NoCR,'Code-Review', 3, Labels),
    S =.. [submit | Labels].
