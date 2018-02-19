## Correção do T3 - 2017/2


Alunos que submeterem correções ao gabarito que forem aceitas receberão 0,5 pontos na prova.

Para submeter uma correção:

- Criar um pull request com um nome que deixe claro qual o bug ("interrupções nao usam barramento durante espera")
- Deve ter uma descrição do bug
- Deve ter os passos para repetir o bug (incluindo qual o repositório do trabalho e o commit que apresenta o bug)
- Deve ter um arquivo anexo com o patch que resolve o bug para a última versão disponível do test.c
- Deve ter o nome do aluno que descobriu o bug
- Deve ter o professor como assignee

Por exemplo,


## Interrupcoes não usam barramento durante espera

Na especificação do trabalho, linha 150, consta que interrupcoes não usam o barramento durante a espera. O caso de teste está esperando que use o barramento.

Passos para repetir:

1. Executa o teste com o meu_repo_si_to_rio, commit 123456. 
2. O resultado "uso de barramento" na linha 150 dá SUCESSO e deveria dar FALHA.

Patch em anexo.

Aluno: Maurício Lima Pilla <pilla@inf.ufpel.edu.br>  


[Pull requests no github](https://help.github.com/articles/about-pull-requests/)
