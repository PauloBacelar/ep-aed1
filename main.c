/* 
  16857617 - Breno Grugnale Frois Lopes
  16895603 - Paulo Guilherme Bacelar Andrade
  16904495 - Vitor Gutierrez Silva
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAMANHO 10000

typedef struct {
	int *itens;
	size_t quantidade;
	size_t capacidade;
	int ultima_linha;
} ReferenciasDeLinhas;

typedef struct EntradaIndice {
	char *palavra;
	int ocorrencias;
	ReferenciasDeLinhas linhas;
	struct EntradaIndice *proximo;
	struct EntradaIndice *esquerda;
	struct EntradaIndice *direita;
} EntradaIndice;

typedef struct {
	char **linhas;
	size_t quantidade;
	size_t capacidade;
} ArmazenadorDeLinhas;

typedef struct {
	EntradaIndice *cabeca;
	size_t palavras_unicas;
	long comparacoes_construcao;
} IndiceLista;

typedef struct {
	EntradaIndice *raiz;
	size_t palavras_unicas;
	long comparacoes_construcao;
} IndiceArvore;


static void *alocar_ou_sair(size_t n) {
	void *p = malloc(n);
	if (!p) {
		exit(1);
	}

	return p;
}

static void *realocar_ou_sair(void *p, size_t n) {
	void *q = realloc(p, n);
	if (!q) {
		exit(1);
	}

	return q;
}


static char *duplicar_string(const char *s) {
	size_t n = strlen(s);
	char *p = (char *)alocar_ou_sair(n + 1);
	memcpy(p, s, n + 1);

	return p;
}

static void refs_linhas_inicializar(ReferenciasDeLinhas *refs) {
	refs->itens = NULL;
	refs->quantidade = 0;
	refs->capacidade = 0;
	refs->ultima_linha = -1;
}

static void refs_linhas_adicionar(ReferenciasDeLinhas *refs, int indice_linha) {
	if (refs->ultima_linha == indice_linha) {
        return;
    }

	if (refs->quantidade == refs->capacidade) {
		refs->capacidade = refs->capacidade ? refs->capacidade * 2 : 8;
		refs->itens = (int *)realocar_ou_sair(refs->itens, refs->capacidade * sizeof(int));
	}

	refs->itens[refs->quantidade++] = indice_linha;
	refs->ultima_linha = indice_linha;
}

static void refs_linhas_liberar(ReferenciasDeLinhas *refs) {
	free(refs->itens);

	refs->itens = NULL;
	refs->quantidade = 0;
	refs->capacidade = 0;
	refs->ultima_linha = -1;
}

static EntradaIndice *entrada_indice_criar(const char *palavra) {
	EntradaIndice *e = (EntradaIndice *)alocar_ou_sair(sizeof(EntradaIndice));
	e->palavra = duplicar_string(palavra);
	e->ocorrencias = 0;
	refs_linhas_inicializar(&e->linhas);
    
	e->proximo = NULL;
	e->esquerda = NULL;
	e->direita = NULL;

	return e;
}

static void entrada_indice_liberar(EntradaIndice *e) {
	if (!e) {
        return;
    }

	free(e->palavra);
	refs_linhas_liberar(&e->linhas);
	free(e);
}


static void armazenador_linhas_inicializar(ArmazenadorDeLinhas *armaz) {
	armaz->linhas = NULL;
	armaz->quantidade = 0;
	armaz->capacidade = 0;
}

static void armazenador_linhas_adicionar(ArmazenadorDeLinhas *armaz, const char *linha) {
	if (armaz->quantidade == armaz->capacidade) {
		armaz->capacidade = armaz->capacidade ? armaz->capacidade * 2 : 32;
		armaz->linhas = (char **)realocar_ou_sair(armaz->linhas, armaz->capacidade * sizeof(char *));
	}

	armaz->linhas[armaz->quantidade++] = duplicar_string(linha);
}


static void armazenador_linhas_liberar(ArmazenadorDeLinhas *armaz) {
	for (size_t i = 0; i < armaz->quantidade; i++) {
		free(armaz->linhas[i]);
	}

	free(armaz->linhas);

	armaz->linhas = NULL;
	armaz->quantidade = 0;
	armaz->capacidade = 0;
}

static int eh_caractere_de_palavra(unsigned char c) {
	return isalnum(c) || (c >= 128);
}

static void normalizar_palavra_busca(const char *s, char *saida, size_t capacidade_saida) {
	if (capacidade_saida == 0) {
        return;
    }

	size_t j = 0;
	for (size_t i = 0; s[i] != '\0'; i++) {
		unsigned char c = (unsigned char)s[i];
		if (!eh_caractere_de_palavra(c)) {
			if (j > 0) {
                break;
            }
			continue;
		}

		if (j + 1 < capacidade_saida) {
			saida[j++] = (char)tolower(c);
		} else {
			break;
		}
	}

	saida[j] = '\0';
}


static EntradaIndice *indice_lista_buscar(IndiceLista *indice, const char *palavra, long *comparacoes) {
	for (EntradaIndice *e = indice->cabeca; e != NULL; e = e->proximo) {
		(*comparacoes)++;

		if (strcmp(palavra, e->palavra) == 0) {
            return e;
        }
	}

	return NULL;
}

static EntradaIndice *indice_lista_obter_ou_inserir(IndiceLista *indice, const char *palavra, long *comparacoes) {
	EntradaIndice *e = indice_lista_buscar(indice, palavra, comparacoes);
	if (e) {
        return e;
    }

	e = entrada_indice_criar(palavra);
	e->proximo = indice->cabeca;
	indice->cabeca = e;
	indice->palavras_unicas++;

	return e;
}

static void indice_lista_liberar(IndiceLista *indice) {
	EntradaIndice *e = indice->cabeca;

	while (e) {
		EntradaIndice *proximo = e->proximo;
		entrada_indice_liberar(e);
		e = proximo;
	}

	indice->cabeca = NULL;
	indice->palavras_unicas = 0;
	indice->comparacoes_construcao = 0;
}

static EntradaIndice *indice_arvore_buscar(IndiceArvore *indice, const char *palavra, long *comparacoes) {
	EntradaIndice *atual = indice->raiz;

	while (atual) {
		(*comparacoes)++;
		int cmp = strcmp(palavra, atual->palavra);
		if (cmp == 0) {
            return atual;
        }

		atual = (cmp < 0) ? atual->esquerda : atual->direita;
	}

	return NULL;
}

static EntradaIndice *indice_arvore_obter_ou_inserir(IndiceArvore *indice, const char *palavra, long *comparacoes) {
	EntradaIndice **atual = &indice->raiz;

	while (*atual) {
		(*comparacoes)++;
		int cmp = strcmp(palavra, (*atual)->palavra);
		if (cmp == 0) {
            return *atual;
        }

		atual = (cmp < 0) ? &(*atual)->esquerda : &(*atual)->direita;
	}

	*atual = entrada_indice_criar(palavra);
	indice->palavras_unicas++;

	return *atual;
}


static int altura_arvore(const EntradaIndice *no) {
	if (!no) {
        return 0;
    }

	int h_esq = altura_arvore(no->esquerda);
	int h_dir = altura_arvore(no->direita);
	return 1 + (h_esq > h_dir ? h_esq : h_dir);
}

static void arvore_liberar_nos(EntradaIndice *no) {
	if (!no) {
        return;
    }

	arvore_liberar_nos(no->esquerda);
	arvore_liberar_nos(no->direita);
	entrada_indice_liberar(no);
}

static void indice_arvore_liberar(IndiceArvore *indice) {
	arvore_liberar_nos(indice->raiz);
	indice->raiz = NULL;
	indice->palavras_unicas = 0;
	indice->comparacoes_construcao = 0;
}

static void indexar_palavras_da_linha_lista(IndiceLista *indice, const char *linha, int indice_linha) {
	char wordbuf[TAMANHO + 1];
	size_t wlen = 0;

	for (size_t i = 0;; i++) {
		unsigned char c = (unsigned char)linha[i];
		if (c != '\0' && eh_caractere_de_palavra(c)) {
			if (wlen < sizeof(wordbuf) - 1) {
				wordbuf[wlen++] = (char)tolower(c);
			}

			continue;
		}

		if (wlen > 0) {
			wordbuf[wlen] = '\0';
			EntradaIndice *e = indice_lista_obter_ou_inserir(indice, wordbuf, &indice->comparacoes_construcao);
			e->ocorrencias++;
			refs_linhas_adicionar(&e->linhas, indice_linha);
			wlen = 0;
		}

		if (c == '\0') {
            break;
        }
	}
}

static void indexar_palavras_da_linha_arvore(IndiceArvore *indice, const char *linha, int indice_linha) {
	char wordbuf[TAMANHO + 1];
	size_t wlen = 0;

	for (size_t i = 0;; i++) {
		unsigned char c = (unsigned char)linha[i];

		if (c != '\0' && eh_caractere_de_palavra(c)) {
			if (wlen < sizeof(wordbuf) - 1) {
				wordbuf[wlen++] = (char)tolower(c);
			}

			continue;
		}

		if (wlen > 0) {
			wordbuf[wlen] = '\0';
			EntradaIndice *e = indice_arvore_obter_ou_inserir(indice, wordbuf, &indice->comparacoes_construcao);
			e->ocorrencias++;
			refs_linhas_adicionar(&e->linhas, indice_linha);
			wlen = 0;
		}

		if (c == '\0') {
            break;
        }
	}
}

int main(int argc, char ** argv){
	if (argc != 3) {
        return 1;
    }

	const char *nome_arquivo = argv[1];
	const char *tipo_indice = argv[2];
	int usar_arvore = 0;

	if (strcmp(tipo_indice, "lista") == 0) {
		usar_arvore = 0;
	} else if (strcmp(tipo_indice, "arvore") == 0) {
		usar_arvore = 1;
	} else {
		return 1;
	}

	FILE *arquivo = fopen(nome_arquivo, "r");

	ArmazenadorDeLinhas texto;
	armazenador_linhas_inicializar(&texto);

	IndiceLista indice_lista;
	indice_lista.cabeca = NULL;
	indice_lista.palavras_unicas = 0;
	indice_lista.comparacoes_construcao = 0;

	IndiceArvore indice_arvore;
	indice_arvore.raiz = NULL;
	indice_arvore.palavras_unicas = 0;
	indice_arvore.comparacoes_construcao = 0;

	char linebuf[TAMANHO + 1];
	while (fgets(linebuf, TAMANHO, arquivo)) {
		char *nl = strrchr(linebuf, '\n');

		if (nl) {
            *nl = '\0';
        }

		size_t len = strlen(linebuf);

		while (len > 0) {
			char c = linebuf[len - 1];
			if (c == ' ' || c == '\t' || c == '\r') {
				linebuf[len - 1] = '\0';
				len--;
			} else {
				break;
			}
		}

		armazenador_linhas_adicionar(&texto, linebuf);
		int indice_linha = (int)texto.quantidade - 1;

		if (usar_arvore) {
			indexar_palavras_da_linha_arvore(&indice_arvore, linebuf, indice_linha);
		} else {
			indexar_palavras_da_linha_lista(&indice_lista, linebuf, indice_linha);
		}
	}

	fclose(arquivo);

	printf("Arquivo: '%s'\n", nome_arquivo);
	printf("Tipo de indice: '%s'\n", tipo_indice);
	printf("Numero de linhas no arquivo: %zu\n", texto.quantidade);
	if (usar_arvore) {
		printf("Total de palavras unicas indexadas: %zu\n", indice_arvore.palavras_unicas);
		printf("Altura da arvore: %d\n", altura_arvore(indice_arvore.raiz));
		printf("Numero de comparacoes realizadas para a construcao do indice: %ld\n", indice_arvore.comparacoes_construcao);
	} else {
		printf("Total de palavras unicas indexadas: %zu\n", indice_lista.palavras_unicas);
		printf("Numero de comparacoes realizadas para a construcao do indice: %ld\n", indice_lista.comparacoes_construcao);
	}

	char cmd[TAMANHO + 1];
	while(1) {
		printf("> ");
		fflush(stdout);

		if (!fgets(cmd, TAMANHO, stdin)) {
            break;
        }

		char *nl = strrchr(cmd, '\n');
		if (nl) {
            *nl = '\0';
        }

		if (strcmp(cmd, "fim") == 0) {
			break;
		}

        if (strncmp(cmd, "busca", 5) == 0 && (cmd[5] == ' ' || cmd[5] == '\t')) {
			char query[TAMANHO + 1];
			normalizar_palavra_busca(cmd + 6, query, sizeof(query));

			if (query[0] == '\0') {
				printf("Opcao invalida!\n");
				continue;
			}

			long comps = 0;
			EntradaIndice *entrada = NULL;
			if (usar_arvore) {
				entrada = indice_arvore_buscar(&indice_arvore, query, &comps);
			} else {
				entrada = indice_lista_buscar(&indice_lista, query, &comps);
			}

			if (!entrada) {
				printf("Palavra '%s' nao encontrada.\n", query);
				printf("Numero de comparacoes: %ld\n", comps);
				continue;
			}

			printf("Existem %d ocorrências da palavra '%s' na(s) seguinte(s) linha(s):\n", entrada->ocorrencias, query);

			for (size_t i = 0; i < entrada->linhas.quantidade; i++) {
				int li = entrada->linhas.itens[i];
				printf("%05d: %s\n", li + 1, texto.linhas[li]);
			}

			printf("Numero de comparacoes: %ld\n", comps);
			continue;
		}

		printf("Opcao invalida!\n");
	}

	armazenador_linhas_liberar(&texto);
	if (usar_arvore) {
        indice_arvore_liberar(&indice_arvore);
    } else {
        indice_lista_liberar(&indice_lista);
    }

	return 0;
}