# Trabalho GB

## Alunos: Eduardo Remião, Matheus Correa e Rafael do Amaral Porciuncula

### Observação

O nome do projeto está como Trabalho GA pois utilizamos ele como base.

### Instruções para executar:

- Para executar é necessário escolher arquitetura x86
- É necessário copiar o arquivo `dependencies/glew-2.1.0/bin/Release/Win32/glew32.dll` para a pasta `Debug`

### Controle da cena:

- Mover posição da câmera: `W`, `A`, `S`, `D`, "Espaço" para subir, e "Shift" para descer
- Rotação da câmera: Movimento do mouse
- Zoom: Scroll do mouse
- Seleção de movimento:
  - 0: Câmera
  - 1 - 9: Objetos (na ordem de declaração no arquivo options)
- Movimento de objetos:
  - Aperte `R` para selecionar tipo de movimento rotação
  - Aperte `T` para selecionar tipo de movimento translação
  - Controles `W`, `A`, `S`, `D`, "Espaço" e "Shift" causarão movimentos de acordo com o tipo de movimento selecionado previamente
- Utilize o arquivo `options.txt` para opções de inicialização de cena
