# VulcanoLang

Uma linguagem de programação simples e dinâmica implementada em C puro.

Inspirada em JavaScript e Lua, com sintaxe limpa, funções de primeira classe, closures e built-ins úteis. Perfeita pra scripts rápidos, experimentos ou só pra zuera low-level.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/language-C-red.svg)]()
[![Stars](https://img.shields.io/github/stars/Lemiossa/VulcanoLang.svg)]()
[![Last Commit](https://img.shields.io/github/last-commit/Lemiossa/VulcanoLang.svg)]()

## Features

- Operações aritméticas e comparadores básicos: `+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`
- Concatenação de strings com `+`
- Comparação de strings com `==` e `!=` (ordem alfabética com `<`/`>` etc ainda não suporta)
- Variáveis com `var`
- Estruturas condicionais: `if`, `else if`, `else`
- Funções com `fn(...) { ... }`
- `return` para retornar valores
- Built-ins:
  - `print(...)`: imprime múltiplos argumentos (strings, números, booleanos, etc)
  - `input(prompt)`: imprime prompt e retorna string digitada pelo usuário
  - `length(str)`: retorna tamanho da string

> Atualmente, scripts retornam o valor da última expressão ou do `return` explícito.

## Quick Start

### Requisitos
- GCC ou Clang
- Make

### Build
```bash
git clone https://github.com/Lemiossa/VulcanoLang.git
cd VulcanoLang
make
```

### Instalar
```bash
git clone https://github.com/Lemiossa/VulcanoLang.git
cd VulcanoLang
make
make install
```

### Rodar um exemplo
```bash
make example  # roda o example.vul que vem no repo
```

Ou rode qualquer arquivo `.vul`:
```bash
./build/bin/vul caminho/para/seu_script.vul
```

## Exemplos

### Hello World interativo
```vul
print("Hello Matheus\n", 3.14);

var entrada = input("Digite algo: ");
print("Sua entrada:", entrada, "\n");

return 0;
```

**Output:**
```
Hello Matheus
 3.140000
Digite algo: Ola
Sua entrada: Ola
```

### If + função
```vul
fn add(a, b) {
    return a + b;
}

var resultado = add(1, 1);

if (resultado == 2) {
    print("Deu bom!");
} else {
    print("Deu ruim...");
}
```

## TODO
- Loops 
- Suporte a ordenação de strings 
- Mais tipos 

## Contribuindo
Pull requests são bem-vindos! Abre issue primeiro pra discutir mudanças grandes.

## Licença
MIT © Matheus Leme Da Silva 
