/*
 * Analizador Lexico, Sintactico y Semantico para el Procesador 8086
 *
 * Recibe un bloque de entrada y lo evalua caracter por caracter usando una
 * estructura ciclica (while) con control de estados basado en switch/case.
 *
 * Restricciones cumplidas:
 *   - No se usa strtok, split ni operadores indexados de expresiones regulares.
 *   - Lectura estrictamente caracter por caracter.
 *
 * Tokens reconocidos:
 *   - Instrucciones: ADD, SUB, INC, DEC
 *   - Registros de 8 bits (ALTO/BAJO): AL, AH, BL, BH, CL, CH, DL, DH
 *   - Comas
 *   - Espacios
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Longitud maxima del bloque de entrada */
#define MAX_INPUT 4096

/* Maximo numero de tokens a almacenar para el HTML */
#define MAX_TOKENS 512

/* Definicion de tipos de token */
typedef enum {
    TOK_INSTRUCCION,
    TOK_REGISTRO,
    TOK_COMA,
    TOK_ESPACIO,
    TOK_DESCONOCIDO,
    TOK_FIN
} TipoToken;

/* Nombres legibles de cada tipo de token */
const char *nombreTipoToken[] = {
    "INSTRUCCION",
    "REGISTRO 8 BITS",
    "COMA",
    "ESPACIO",
    "DESCONOCIDO",
    "FIN DE LINEA"
};

/* ---- Tabla de instrucciones validas ---- */
static const char *instrucciones[] = {
    "ADD", "SUB", "INC", "DEC"
};
#define NUM_INSTRUCCIONES (sizeof(instrucciones) / sizeof(instrucciones[0]))

/* ---- Tabla de registros de 8 bits (ALTO/BAJO) ---- */
static const char *registros8[] = {
    "AL", "AH", "BL", "BH", "CL", "CH", "DL", "DH"
};
#define NUM_REGISTROS8 (sizeof(registros8) / sizeof(registros8[0]))

/* ---- Estados del automata finito ---- */
typedef enum {
    EST_INICIO,
    EST_LEYENDO_PAL
} Estado;

/* Estructura que representa un token reconocido */
typedef struct {
    TipoToken tipo;
    char      lexema[32];
    int       linea;
} Token;

/* Arreglo global para almacenar tokens y generar el HTML */
static Token tokensHtml[MAX_TOKENS];
static int   numTokensHtml = 0;

/* Almacen de resultados semanticos (declarado aqui para resetTokens) */
static int numResultadosSem = 0;

/* Guarda un token en el arreglo para el HTML */
static void guardarToken(TipoToken tipo, const char *lexema, int linea)
{
    if (numTokensHtml < MAX_TOKENS) {
        tokensHtml[numTokensHtml].tipo  = tipo;
        tokensHtml[numTokensHtml].linea = linea;
        strncpy(tokensHtml[numTokensHtml].lexema, lexema,
                sizeof(tokensHtml[numTokensHtml].lexema) - 1);
        tokensHtml[numTokensHtml].lexema[sizeof(tokensHtml[numTokensHtml].lexema) - 1] = '\0';
        numTokensHtml++;
    }
}

/*
 * Verifica si una palabra (en mayusculas) es una instruccion valida.
 */
static int esInstruccion(const char *palabra)
{
    size_t i;
    for (i = 0; i < NUM_INSTRUCCIONES; i++) {
        if (strcmp(palabra, instrucciones[i]) == 0)
            return 1;
    }
    return 0;
}

/*
 * Verifica si una palabra (en mayusculas) es un registro de 8 bits valido.
 */
static int esRegistro8(const char *palabra)
{
    size_t i;
    for (i = 0; i < NUM_REGISTROS8; i++) {
        if (strcmp(palabra, registros8[i]) == 0)
            return 1;
    }
    return 0;
}

/*
 * Devuelve el indice (0-7) de un registro de 8 bits dentro de la tabla
 * registros8[], o -1 si el nombre no corresponde a ningun registro.
 */
static int indiceReg(const char *nombre)
{
    int r;
    for (r = 0; r < 8; r++) {
        if (strcmp(nombre, registros8[r]) == 0) return r;
    }
    return -1;
}

/*
 * Analizador lexico principal.
 */
static void analizar(const char *entrada)
{
    int    i = 0;
    int    linea = 1;
    Estado estado = EST_INICIO;
    char   palabra[32];
    int    palLen;
    char   c;

    printf("====================================================\n");
    printf("  ANALIZADOR LEXICO - PROCESADOR 8086\n");
    printf("====================================================\n");
    printf("Entrada recibida:\n%s\n", entrada);
    printf("----------------------------------------------------\n");
    printf("Linea | Tipo de Token        | Lexema\n");
    printf("----------------------------------------------------\n");

    while (entrada[i] != '\0') {

        c = entrada[i];

        switch (estado) {

        case EST_INICIO:
            switch (c) {
            case ' ': case '\t':
                printf(" %3d  | %-20s | \"%c\"\n",
                       linea, nombreTipoToken[TOK_ESPACIO],
                       (c == '\t') ? ' ' : ' ');
                guardarToken(TOK_ESPACIO, " ", linea);
                break;
            case ',':
                printf(" %3d  | %-20s | \",\"\n",
                       linea, nombreTipoToken[TOK_COMA]);
                guardarToken(TOK_COMA, ",", linea);
                break;
            case '\n':
                printf(" %3d  | %-20s | \"\\n\"\n",
                       linea, nombreTipoToken[TOK_FIN]);
                guardarToken(TOK_FIN, "\\n", linea);
                linea++;
                break;
            case '\r':
                break;
            default:
                if (isalpha((unsigned char)c)) {
                    palLen = 0;
                    palabra[palLen++] = (char)toupper((unsigned char)c);
                    estado = EST_LEYENDO_PAL;
                } else {
                    printf(" %3d  | %-20s | '%c' (0x%02X)\n",
                           linea, nombreTipoToken[TOK_DESCONOCIDO],
                           c, (unsigned char)c);
                    {
                        char tmp[8];
                        snprintf(tmp, sizeof(tmp), "%c", c);
                        guardarToken(TOK_DESCONOCIDO, tmp, linea);
                    }
                }
                break;
            }
            break;

        case EST_LEYENDO_PAL:
            if (isalpha((unsigned char)c)) {
                if (palLen < (int)sizeof(palabra) - 1)
                    palabra[palLen++] = (char)toupper((unsigned char)c);
            } else {
                palabra[palLen] = '\0';

                Token tok;
                tok.linea = linea;
                strcpy(tok.lexema, palabra);

                if (esInstruccion(palabra))
                    tok.tipo = TOK_INSTRUCCION;
                else if (esRegistro8(palabra))
                    tok.tipo = TOK_REGISTRO;
                else
                    tok.tipo = TOK_DESCONOCIDO;

                printf(" %3d  | %-20s | \"%s\"\n",
                       tok.linea, nombreTipoToken[tok.tipo], tok.lexema);
                guardarToken(tok.tipo, tok.lexema, tok.linea);

                estado = EST_INICIO;
                i--;
            }
            break;

        default:
            estado = EST_INICIO;
            break;
        }

        i++;
    }

    if (estado == EST_LEYENDO_PAL) {
        palabra[palLen] = '\0';

        Token tok;
        tok.linea = linea;
        strcpy(tok.lexema, palabra);

        if (esInstruccion(palabra))
            tok.tipo = TOK_INSTRUCCION;
        else if (esRegistro8(palabra))
            tok.tipo = TOK_REGISTRO;
        else
            tok.tipo = TOK_DESCONOCIDO;

        printf(" %3d  | %-20s | \"%s\"\n",
               tok.linea, nombreTipoToken[tok.tipo], tok.lexema);
        guardarToken(tok.tipo, tok.lexema, tok.linea);
    }

    printf("====================================================\n");
    printf("Fin del analisis lexico.\n");
}

/* Escapa caracteres HTML especiales en una cadena */
static void escaparHtml(FILE *f, const char *s)
{
    while (*s) {
        switch (*s) {
        case '<':  fputs("&lt;",  f); break;
        case '>':  fputs("&gt;",  f); break;
        case '&':  fputs("&amp;", f); break;
        case '"':  fputs("&quot;", f); break;
        default:   fputc(*s, f);      break;
        }
        s++;
    }
}

/* Colores por tipo de token (clases CSS) */
static const char *claseCss(TipoToken t)
{
    switch (t) {
    case TOK_INSTRUCCION: return "tok-inst";
    case TOK_REGISTRO:    return "tok-reg";
    case TOK_COMA:        return "tok-coma";
    case TOK_ESPACIO:     return "tok-esp";
    case TOK_DESCONOCIDO: return "tok-unk";
    case TOK_FIN:         return "tok-fin";
    default:              return "tok-unk";
    }
}

/* Cuenta cuantos tokens hay de cada tipo */
static void contarTokens(int conteo[6])
{
    int k;
    for (k = 0; k < 6; k++) conteo[k] = 0;
    for (k = 0; k < numTokensHtml; k++)
        conteo[tokensHtml[k].tipo]++;
}

/* ---- CSS base compartido por las tres paginas ---- */
static const char *cssBase =
    "  * { margin: 0; padding: 0; box-sizing: border-box; }\n"
    "  body {\n"
    "    font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;\n"
    "    background: #0f172a;\n"
    "    color: #e2e8f0;\n"
    "    min-height: 100vh;\n"
    "    padding: 2rem 1rem;\n"
    "  }\n"
    "  .container { max-width: 960px; margin: 0 auto; }\n"
    "  header { text-align: center; margin-bottom: 2rem; }\n"
    "  header h1 {\n"
    "    font-size: 2rem; font-weight: 700;\n"
    "    margin-bottom: 0.5rem;\n"
    "  }\n"
    "  header p { color: #94a3b8; font-size: 0.95rem; }\n"
    "  .section { margin-bottom: 2rem; }\n"
    "  .section-title {\n"
    "    font-size: 1.1rem; font-weight: 600;\n"
    "    margin-bottom: 1rem;\n"
    "    padding-bottom: 0.5rem;\n"
    "    border-bottom: 1px solid #1e293b;\n"
    "  }\n"
    "  .input-box {\n"
    "    background: #1e293b; border: 1px solid #334155;\n"
    "    border-radius: 8px; padding: 1.25rem;\n"
    "    font-family: 'Courier New', monospace;\n"
    "    font-size: 0.95rem; line-height: 1.8;\n"
    "    white-space: pre-wrap; color: #cbd5e1;\n"
    "  }\n"
    "  .stats {\n"
    "    display: grid;\n"
    "    grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));\n"
    "    gap: 1rem; margin-bottom: 2rem;\n"
    "  }\n"
    "  .stat-card {\n"
    "    background: #1e293b; border: 1px solid #334155;\n"
    "    border-radius: 8px; padding: 1rem;\n"
    "    text-align: center;\n"
    "  }\n"
    "  .stat-card .num { font-size: 1.8rem; font-weight: 700; }\n"
    "  .stat-card .lbl {\n"
    "    font-size: 0.75rem; color: #94a3b8;\n"
    "    text-transform: uppercase; letter-spacing: 0.05em;\n"
    "    margin-top: 0.25rem;\n"
    "  }\n"
    "  table {\n"
    "    width: 100%%; border-collapse: collapse;\n"
    "    background: #1e293b;\n"
    "    border-radius: 8px; overflow: hidden;\n"
    "    border: 1px solid #334155;\n"
    "  }\n"
    "  th {\n"
    "    background: #334155; color: #e2e8f0;\n"
    "    padding: 0.75rem 1rem; text-align: left;\n"
    "    font-size: 0.85rem; font-weight: 600;\n"
    "    text-transform: uppercase; letter-spacing: 0.05em;\n"
    "  }\n"
    "  td {\n"
    "    padding: 0.6rem 1rem;\n"
    "    border-bottom: 1px solid #1e293b;\n"
    "    font-size: 0.9rem;\n"
    "  }\n"
    "  tr:hover td { background: #283548; }\n"
    "  .badge {\n"
    "    display: inline-block; padding: 0.2rem 0.6rem;\n"
    "    border-radius: 4px; font-size: 0.8rem;\n"
    "    font-weight: 600; white-space: nowrap;\n"
    "  }\n"
    "  .lexema {\n"
    "    font-family: 'Courier New', monospace;\n"
    "    color: #cbd5e1;\n"
    "  }\n"
    "  .linea { color: #64748b; font-weight: 600; }\n"
    "  footer {\n"
    "    text-align: center; color: #475569;\n"
    "    font-size: 0.8rem; margin-top: 2rem;\n"
    "  }\n"
    "  .textarea-box {\n"
    "    width: 100%%; min-height: 180px;\n"
    "    background: #1e293b; border: 1px solid #334155;\n"
    "    border-radius: 8px; padding: 1rem;\n"
    "    font-family: 'Courier New', monospace;\n"
    "    font-size: 0.95rem; line-height: 1.6;\n"
    "    color: #e2e8f0; resize: vertical; outline: none;\n"
    "  }\n"
    "  .btn {\n"
    "    color: #fff; border: none; border-radius: 8px;\n"
    "    padding: 0.7rem 2rem; font-size: 1rem; font-weight: 600;\n"
    "    cursor: pointer; margin-top: 1rem; transition: opacity 0.2s;\n"
    "  }\n"
    "  .btn:hover { opacity: 0.85; }\n"
    "  .btn:active { transform: scale(0.97); }\n"
    "  .btn-limpiar { background: #334155; margin-left: 0.5rem; }\n"
    "  .btn-limpiar:hover { background: #3e4b66; }\n"
    "  .result-area { margin-top: 1.5rem; }\n"
    "  .hint { color: #64748b; font-size: 0.85rem; margin-top: 0.5rem; }\n"
    "  .nav-links { margin-bottom: 1rem; }\n"
    "  .nav-link {\n"
    "    display: inline-block; text-decoration: none;\n"
    "    font-size: 0.9rem; margin-right: 1rem;\n"
    "    padding: 0.3rem 0.8rem; border-radius: 6px;\n"
    "    border: 1px solid #334155; background: #1e293b;\n"
    "  }\n"
    "  .nav-link:hover { background: #283548; }\n"
    "  .grammar-box {\n"
    "    background: #1e293b; border: 1px solid #334155;\n"
    "    border-radius: 8px; padding: 1.25rem;\n"
    "    font-family: 'Courier New', monospace;\n"
    "    font-size: 0.9rem; line-height: 1.8; color: #cbd5e1;\n"
    "  }\n"
    "  .grammar-box .prod { font-weight: 600; }\n"
    "  .grammar-box .term { color: #38bdf8; }\n"
    "  @media (max-width: 640px) {\n"
    "    header h1 { font-size: 1.5rem; }\n"
    "    .stats { grid-template-columns: repeat(2, 1fr); }\n"
    "    th, td { padding: 0.5rem 0.6rem; font-size: 0.8rem; }\n"
    "  }\n";

/* ---- JavaScript base compartido: analizador lexico ---- */
static const char *jsLexico =
    "const instrucciones = ['ADD','SUB','INC','DEC'];\n"
    "const registros8 = ['AL','AH','BL','BH','CL','CH','DL','DH'];\n"
    "const nombreTipoToken = ['INSTRUCCION','REGISTRO 8 BITS','COMA','ESPACIO','DESCONOCIDO','FIN DE LINEA'];\n"
    "const claseCss = ['tok-inst','tok-reg','tok-coma','tok-esp','tok-unk','tok-fin'];\n"
    "\n"
    "function esInstruccion(p) { return instrucciones.indexOf(p) !== -1; }\n"
    "function esRegistro8(p) { return registros8.indexOf(p) !== -1; }\n"
    "function esLetra(c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }\n"
    "function escaparHtml(s) {\n"
    "    return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/\"/g,'&quot;');\n"
    "}\n"
    "\n"
    "function analizarLexico(entrada) {\n"
    "    const tokens = [];\n"
    "    let i = 0, linea = 1;\n"
    "    let estado = 0;\n"
    "    let palabra = '', palLen = 0;\n"
    "    while (i < entrada.length) {\n"
    "        const c = entrada[i];\n"
    "        if (estado === 0) {\n"
    "            if (c === ' ' || c === '\\t') {\n"
    "                tokens.push({tipo:3, lexema:' ', linea:linea});\n"
    "            } else if (c === ',') {\n"
    "                tokens.push({tipo:2, lexema:',', linea:linea});\n"
    "            } else if (c === '\\n') {\n"
    "                tokens.push({tipo:5, lexema:'\\\\n', linea:linea});\n"
    "                linea++;\n"
    "            } else if (c === '\\r') {\n"
    "            } else if (esLetra(c)) {\n"
    "                palabra = c.toUpperCase(); palLen = 1; estado = 1;\n"
    "            } else {\n"
    "                const hex = c.charCodeAt(0).toString(16).toUpperCase().padStart(2,'0');\n"
    "                tokens.push({tipo:4, lexema:\"'\" + c + \"' (0x\" + hex + \")\", linea:linea});\n"
    "            }\n"
    "        } else {\n"
    "            if (esLetra(c)) {\n"
    "                if (palLen < 31) { palabra += c.toUpperCase(); palLen++; }\n"
    "            } else {\n"
    "                let tipo;\n"
    "                if (esInstruccion(palabra)) tipo = 0;\n"
    "                else if (esRegistro8(palabra)) tipo = 1;\n"
    "                else tipo = 4;\n"
    "                tokens.push({tipo:tipo, lexema:palabra, linea:linea});\n"
    "                estado = 0; i--;\n"
    "            }\n"
    "        }\n"
    "        i++;\n"
    "    }\n"
    "    if (estado === 1) {\n"
    "        let tipo;\n"
    "        if (esInstruccion(palabra)) tipo = 0;\n"
    "        else if (esRegistro8(palabra)) tipo = 1;\n"
    "        else tipo = 4;\n"
    "        tokens.push({tipo:tipo, lexema:palabra, linea:linea});\n"
    "    }\n"
    "    return tokens;\n"
    "}\n";

/*
 * Genera el HTML del analizador lexico.
 */
static void generarHtml(const char *entrada, const char *archivo, const char *titulo)
{
    FILE *f = fopen(archivo, "w");
    int  conteo[6];
    int  k;

    if (!f) {
        fprintf(stderr, "Error: no se pudo crear %s\n", archivo);
        return;
    }

    contarTokens(conteo);

    fprintf(f,
        "<!DOCTYPE html>\n"
        "<html lang=\"es\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "<title>Analizador Lexico - Procesador 8086</title>\n"
        "<style>\n"
        "  header h1 {\n"
        "    background: linear-gradient(135deg, #38bdf8, #0ea5e9);\n"
        "    -webkit-background-clip: text; -webkit-text-fill-color: transparent;\n"
        "    background-clip: text;\n"
        "  }\n"
        "  .section-title { color: #38bdf8; }\n"
        "  .stat-inst .num { color: #38bdf8; }\n"
        "  .stat-reg  .num { color: #34d399; }\n"
        "  .stat-coma .num { color: #fbbf24; }\n"
        "  .stat-esp  .num { color: #94a3b8; }\n"
        "  .stat-fin  .num { color: #f87171; }\n"
        "  .stat-unk  .num { color: #f87171; }\n"
        "  .stat-tot  .num { color: #c084fc; }\n"
        "  .tok-inst { background: rgba(56,189,248,0.15); color: #38bdf8; border: 1px solid rgba(56,189,248,0.3); }\n"
        "  .tok-reg  { background: rgba(52,211,153,0.15); color: #34d399; border: 1px solid rgba(52,211,153,0.3); }\n"
        "  .tok-coma { background: rgba(251,191,36,0.15); color: #fbbf24; border: 1px solid rgba(251,191,36,0.3); }\n"
        "  .tok-esp  { background: rgba(148,163,184,0.15); color: #94a3b8; border: 1px solid rgba(148,163,184,0.3); }\n"
        "  .tok-fin  { background: rgba(248,113,113,0.15); color: #f87171; border: 1px solid rgba(248,113,113,0.3); }\n"
        "  .tok-unk  { background: rgba(248,113,113,0.15); color: #f87171; border: 1px solid rgba(248,113,113,0.3); }\n"
        "  .btn { background: linear-gradient(135deg, #0ea5e9, #38bdf8); }\n"
        "  .textarea-box:focus { border-color: #38bdf8; }\n"
        "  .nav-link.lex { background: #0ea5e9; color: #fff; border-color: #0ea5e9; }\n"
        "%s"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"container\">\n"
        "  <header>\n"
        "    <h1>Analizador Lexico - Procesador 8086</h1>\n"
        "    <p>%s</p>\n"
        "  </header>\n"
        , cssBase, titulo);

    fprintf(f,
        "  <div class=\"nav-links\">\n"
        "    <a class=\"nav-link lex\" href=\"#\">Lexico</a>\n"
        "    <a class=\"nav-link\" href=\"resultado_sintactico.html\">Sintactico</a>\n"
        "    <a class=\"nav-link\" href=\"resultado_semantico.html\">Semantico</a>\n"
        "  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Codigo de Entrada</div>\n"
        "    <div class=\"input-box\">");
    escaparHtml(f, entrada);
    fprintf(f, "</div>\n  </div>\n"

        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Resumen de Tokens</div>\n"
        "    <div class=\"stats\">\n"
        "      <div class=\"stat-card stat-inst\"><div class=\"num\">%d</div><div class=\"lbl\">Instrucciones</div></div>\n"
        "      <div class=\"stat-card stat-reg\"><div class=\"num\">%d</div><div class=\"lbl\">Registros 8b</div></div>\n"
        "      <div class=\"stat-card stat-coma\"><div class=\"num\">%d</div><div class=\"lbl\">Comas</div></div>\n"
        "      <div class=\"stat-card stat-esp\"><div class=\"num\">%d</div><div class=\"lbl\">Espacios</div></div>\n"
        "      <div class=\"stat-card stat-fin\"><div class=\"num\">%d</div><div class=\"lbl\">Fin de Linea</div></div>\n"
        "      <div class=\"stat-card stat-unk\"><div class=\"num\">%d</div><div class=\"lbl\">Desconocidos</div></div>\n"
        "      <div class=\"stat-card stat-tot\"><div class=\"num\">%d</div><div class=\"lbl\">Total Tokens</div></div>\n"
        "    </div>\n"
        "  </div>\n"
        ,
        conteo[TOK_INSTRUCCION], conteo[TOK_REGISTRO], conteo[TOK_COMA],
        conteo[TOK_ESPACIO], conteo[TOK_FIN], conteo[TOK_DESCONOCIDO], numTokensHtml);

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Tabla de Tokens</div>\n"
        "    <table>\n"
        "      <thead>\n"
        "        <tr><th>Linea</th><th>Tipo de Token</th><th>Lexema</th></tr>\n"
        "      </thead>\n"
        "      <tbody>\n");

    for (k = 0; k < numTokensHtml; k++) {
        fprintf(f,
            "        <tr><td class=\"linea\">%d</td>"
            "<td><span class=\"badge %s\">%s</span></td>"
            "<td class=\"lexema\">&quot;",
            tokensHtml[k].linea,
            claseCss(tokensHtml[k].tipo),
            nombreTipoToken[tokensHtml[k].tipo]);
        escaparHtml(f, tokensHtml[k].lexema);
        fprintf(f, "&quot;</td></tr>\n");
    }

    fprintf(f,
        "      </tbody>\n"
        "    </table>\n"
        "  </div>\n"
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Modo Personalizado - Escriba su propio bloque</div>\n"
        "    <p class=\"hint\">Instrucciones validas: ADD, SUB, INC, DEC. Registros de 8 bits: AL, AH, BL, BH, CL, CH, DL, DH. Cualquier otro caracter se marcara como DESCONOCIDO.</p>\n"
        "    <textarea class=\"textarea-box\" id=\"entradaUsuario\" placeholder=\"Ej: ADD AL, BH\"></textarea>\n"
        "    <div>\n"
        "      <button class=\"btn\" onclick=\"ejecutarAnalisis()\">Analizar</button>\n"
        "      <button class=\"btn btn-limpiar\" onclick=\"limpiar()\">Limpiar</button>\n"
        "    </div>\n"
        "    <div class=\"result-area\" id=\"resultadoUsuario\">\n"
        "      <p class=\"hint\">Presione \"Analizar\" para ver los tokens.</p>\n"
        "    </div>\n"
        "  </div>\n"
        "  <footer>Analizador Lexico 8086 - Generado automaticamente desde C</footer>\n"
        "</div>\n");

    fputs("<script>\n", f);
    fputs(jsLexico, f);
    fputs(
        "function renderResultados(tokens, contenedorId) {\n"
        "    const cont = document.getElementById(contenedorId);\n"
        "    if (tokens.length === 0) {\n"
        "        cont.innerHTML = '<p class=\"hint\">No se encontraron tokens.</p>';\n"
        "        return;\n"
        "    }\n"
        "    const conteo = [0,0,0,0,0,0];\n"
        "    let k;\n"
        "    for (k = 0; k < tokens.length; k++) conteo[tokens[k].tipo]++;\n"
        "    let html = '<div class=\"section\"><div class=\"section-title\">Resumen de Tokens</div>';\n"
        "    html += '<div class=\"stats\">';\n"
        "    html += '<div class=\"stat-card stat-inst\"><div class=\"num\">'+conteo[0]+'</div><div class=\"lbl\">Instrucciones</div></div>';\n"
        "    html += '<div class=\"stat-card stat-reg\"><div class=\"num\">'+conteo[1]+'</div><div class=\"lbl\">Registros 8b</div></div>';\n"
        "    html += '<div class=\"stat-card stat-coma\"><div class=\"num\">'+conteo[2]+'</div><div class=\"lbl\">Comas</div></div>';\n"
        "    html += '<div class=\"stat-card stat-esp\"><div class=\"num\">'+conteo[3]+'</div><div class=\"lbl\">Espacios</div></div>';\n"
        "    html += '<div class=\"stat-card stat-fin\"><div class=\"num\">'+conteo[5]+'</div><div class=\"lbl\">Fin de Linea</div></div>';\n"
        "    html += '<div class=\"stat-card stat-unk\"><div class=\"num\">'+conteo[4]+'</div><div class=\"lbl\">Desconocidos</div></div>';\n"
        "    html += '<div class=\"stat-card stat-tot\"><div class=\"num\">'+tokens.length+'</div><div class=\"lbl\">Total Tokens</div></div>';\n"
        "    html += '</div></div>';\n"
        "    html += '<div class=\"section\"><div class=\"section-title\">Tabla de Tokens</div>';\n"
        "    html += '<table><thead><tr><th>Linea</th><th>Tipo de Token</th><th>Lexema</th></tr></thead><tbody>';\n"
        "    for (k = 0; k < tokens.length; k++) {\n"
        "        const t = tokens[k];\n"
        "        html += '<tr><td class=\"linea\">'+t.linea+'</td>';\n"
        "        html += '<td><span class=\"badge '+claseCss[t.tipo]+'\">'+nombreTipoToken[t.tipo]+'</span></td>';\n"
        "        html += '<td class=\"lexema\">&quot;'+escaparHtml(t.lexema)+'&quot;</td></tr>';\n"
        "    }\n"
        "    html += '</tbody></table></div>';\n"
        "    cont.innerHTML = html;\n"
        "}\n"
        "function ejecutarAnalisis() {\n"
        "    const texto = document.getElementById('entradaUsuario').value;\n"
        "    const tokens = analizarLexico(texto);\n"
        "    renderResultados(tokens, 'resultadoUsuario');\n"
        "}\n"
        "function limpiar() {\n"
        "    document.getElementById('entradaUsuario').value = '';\n"
        "    document.getElementById('resultadoUsuario').innerHTML = '<p class=\"hint\">Presione \\\"Analizar\\\" para ver los tokens.</p>';\n"
        "}\n"
        "</script>\n"
        "</body>\n"
        "</html>\n", f);

    fclose(f);
    printf("\nArchivo HTML generado: %s\n", archivo);
}

/* ---- Analizador Sintactico ---- */
/*
 toma los tokens que ya reconoció el analizador léxico y 
 comprueba si cada línea sigue una gramática muy simple.
*/

/*
separa tokens por línea
revisa la forma de la línea
valida con reglas fijas*/
typedef struct {
    int  linea;
    int  correcto;
    char mensaje[160];
    char instruccion[128];
} ResultadoSintactico;

static ResultadoSintactico resultadosSint[MAX_TOKENS];
static int numResultadosSint = 0;

/* Reinicia el almacen de tokens para un nuevo analisis */
static void resetTokens(void)
{
    numTokensHtml = 0;
    numResultadosSint = 0;
    numResultadosSem = 0;
}

//la funcion recorre los tokens generados por el analizador lexico y verifica si 
//la sintaxis es correcta

static void analizarSintactico(void)
{
    int k = 0;
    int lineaActual;
    Token tokensLinea[16];
    int  nTok;
    int  j;
    char instr[128];

    printf("\n");
    printf("====================================================\n");
    printf("  ANALIZADOR SINTACTICO - PROCESADOR 8086\n");
    printf("====================================================\n");
    printf("Linea | Resultado  | Descripcion\n");
    printf("----------------------------------------------------\n");

    //ciclo externo donde se agrupa los tokens en lineas para poder analizarlos
    while (k < numTokensHtml) {
        lineaActual = tokensHtml[k].linea;

        nTok = 0;

        //ciclo interno donde se agrupa los tokens en lineas para poder analizarlos
        while (k < numTokensHtml && tokensHtml[k].linea == lineaActual) {
            if (tokensHtml[k].tipo != TOK_ESPACIO &&
                tokensHtml[k].tipo != TOK_FIN) {
                if (nTok < 16) {
                    tokensLinea[nTok++] = tokensHtml[k];
                }
            }
            k++;
        }

        instr[0] = '\0';
        for (j = 0; j < nTok; j++) {
            if (j > 0) strcat(instr, " ");
            strcat(instr, tokensLinea[j].lexema);
        }

        if (numResultadosSint < MAX_TOKENS) {
            resultadosSint[numResultadosSint].linea = lineaActual;
            strcpy(resultadosSint[numResultadosSint].instruccion, instr);

            if (nTok == 0) {
                resultadosSint[numResultadosSint].correcto = 0;
                snprintf(resultadosSint[numResultadosSint].mensaje,
                         sizeof(resultadosSint[numResultadosSint].mensaje),
                         "Linea vacia - no hay instruccion");
            }
            else if (tokensLinea[0].tipo != TOK_INSTRUCCION) {
                resultadosSint[numResultadosSint].correcto = 0;
                snprintf(resultadosSint[numResultadosSint].mensaje,
                         sizeof(resultadosSint[numResultadosSint].mensaje),
                         "Error: se esperaba una INSTRUCCION al inicio, se encontro '%s' (%s)",
                         tokensLinea[0].lexema,
                         nombreTipoToken[tokensLinea[0].tipo]);
            }
            else if (strcmp(tokensLinea[0].lexema, "ADD") == 0 ||
                     strcmp(tokensLinea[0].lexema, "SUB") == 0) {

                if (nTok != 4) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: %s requiere 2 operandos (INSTRUCCION REGISTRO, REGISTRO) - se encontraron %d tokens",
                             tokensLinea[0].lexema, nTok);
                }
                else if (tokensLinea[1].tipo != TOK_REGISTRO) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: despues de %s se esperaba un REGISTRO, se encontro '%s' (%s)",
                             tokensLinea[0].lexema,
                             tokensLinea[1].lexema,
                             nombreTipoToken[tokensLinea[1].tipo]);
                }
                else if (tokensLinea[2].tipo != TOK_COMA) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: se esperaba una COMA despues de '%s', se encontro '%s' (%s)",
                             tokensLinea[1].lexema,
                             tokensLinea[2].lexema,
                             nombreTipoToken[tokensLinea[2].tipo]);
                }
                else if (tokensLinea[3].tipo != TOK_REGISTRO) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: despues de la COMA se esperaba un REGISTRO, se encontro '%s' (%s)",
                             tokensLinea[3].lexema,
                             nombreTipoToken[tokensLinea[3].tipo]);
                }
                else {
                    resultadosSint[numResultadosSint].correcto = 1;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Sintaxis correcta: %s %s, %s",
                             tokensLinea[0].lexema,
                             tokensLinea[1].lexema,
                             tokensLinea[3].lexema);
                }
            }
            else if (strcmp(tokensLinea[0].lexema, "INC") == 0 ||
                     strcmp(tokensLinea[0].lexema, "DEC") == 0) {

                if (nTok != 2) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: %s requiere 1 operando (INSTRUCCION REGISTRO) - se encontraron %d tokens",
                             tokensLinea[0].lexema, nTok);
                }
                else if (tokensLinea[1].tipo != TOK_REGISTRO) {
                    resultadosSint[numResultadosSint].correcto = 0;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Error: despues de %s se esperaba un REGISTRO, se encontro '%s' (%s)",
                             tokensLinea[0].lexema,
                             tokensLinea[1].lexema,
                             nombreTipoToken[tokensLinea[1].tipo]);
                }
                else {
                    resultadosSint[numResultadosSint].correcto = 1;
                    snprintf(resultadosSint[numResultadosSint].mensaje,
                             sizeof(resultadosSint[numResultadosSint].mensaje),
                             "Sintaxis correcta: %s %s",
                             tokensLinea[0].lexema,
                             tokensLinea[1].lexema);
                }
            }
            else {
                resultadosSint[numResultadosSint].correcto = 0;
                snprintf(resultadosSint[numResultadosSint].mensaje,
                         sizeof(resultadosSint[numResultadosSint].mensaje),
                         "Error: instruccion '%s' no reconocida en la gramatica",
                         tokensLinea[0].lexema);
            }

            printf(" %3d  | %-9s | %s\n",
                   resultadosSint[numResultadosSint].linea,
                   resultadosSint[numResultadosSint].correcto ? "OK" : "ERROR",
                   resultadosSint[numResultadosSint].mensaje);

            numResultadosSint++;
        }
    }

    {
        int ok = 0, err = 0;
        //ciclo para construir la intruccion
        for (j = 0; j < numResultadosSint; j++) {
            if (resultadosSint[j].correcto) ok++;
            else err++;
        }
        printf("----------------------------------------------------\n");
        printf("Resumen: %d correctas, %d con error, %d total\n",
               ok, err, numResultadosSint);
        printf("====================================================\n");
        printf("Fin del analisis sintactico.\n");
    }
}

/*
 * Genera el HTML del analizador sintactico.
 */
static void generarHtmlSintactico(const char *entrada, const char *archivo,
                                  const char *titulo)
{
    FILE *f = fopen(archivo, "w");
    int  k;
    int  ok = 0, err = 0;

    if (!f) {
        fprintf(stderr, "Error: no se pudo crear %s\n", archivo);
        return;
    }

    for (k = 0; k < numResultadosSint; k++) {
        if (resultadosSint[k].correcto) ok++;
        else err++;
    }

    fprintf(f,
        "<!DOCTYPE html>\n"
        "<html lang=\"es\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "<title>Analizador Sintactico - Procesador 8086</title>\n"
        "<style>\n"
        "  header h1 {\n"
        "    background: linear-gradient(135deg, #34d399, #10b981);\n"
        "    -webkit-background-clip: text; -webkit-text-fill-color: transparent;\n"
        "    background-clip: text;\n"
        "  }\n"
        "  .section-title { color: #34d399; }\n"
        "  .stat-ok  .num { color: #34d399; }\n"
        "  .stat-err .num { color: #f87171; }\n"
        "  .stat-tot .num { color: #c084fc; }\n"
        "  .badge-ok {\n"
        "    display: inline-block; padding: 0.2rem 0.6rem;\n"
        "    border-radius: 4px; font-size: 0.8rem; font-weight: 600;\n"
        "    background: rgba(52,211,153,0.15); color: #34d399;\n"
        "    border: 1px solid rgba(52,211,153,0.3);\n"
        "  }\n"
        "  .badge-err {\n"
        "    display: inline-block; padding: 0.2rem 0.6rem;\n"
        "    border-radius: 4px; font-size: 0.8rem; font-weight: 600;\n"
        "    background: rgba(248,113,113,0.15); color: #f87171;\n"
        "    border: 1px solid rgba(248,113,113,0.3);\n"
        "  }\n"
        "  .instr { font-family: 'Courier New', monospace; color: #cbd5e1; }\n"
        "  .desc-ok  { color: #94a3b8; }\n"
        "  .desc-err { color: #f87171; }\n"
        "  .grammar-box .prod { color: #34d399; }\n"
        "  .btn { background: linear-gradient(135deg, #10b981, #34d399); }\n"
        "  .textarea-box:focus { border-color: #34d399; }\n"
        "  .nav-link.sint { background: #10b981; color: #fff; border-color: #10b981; }\n"
        "%s"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"container\">\n"
        "  <header>\n"
        "    <h1>Analizador Sintactico - Procesador 8086</h1>\n"
        "    <p>%s</p>\n"
        "  </header>\n"
        , cssBase, titulo);

    fprintf(f,
        "  <div class=\"nav-links\">\n"
        "    <a class=\"nav-link\" href=\"resultado_lexico.html\">Lexico</a>\n"
        "    <a class=\"nav-link sint\" href=\"#\">Sintactico</a>\n"
        "    <a class=\"nav-link\" href=\"resultado_semantico.html\">Semantico</a>\n"
        "  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Codigo de Entrada</div>\n"
        "    <div class=\"input-box\">");
    escaparHtml(f, entrada);
    fprintf(f, "</div>\n  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Gramatica Definida</div>\n"
        "    <div class=\"grammar-box\">\n"
        "      <span class=\"prod\">instruccion</span> &rarr; <span class=\"term\">ADD</span> <span class=\"prod\">registro</span> <span class=\"term\">,</span> <span class=\"prod\">registro</span><br>\n"
        "      <span class=\"prod\">instruccion</span> &rarr; <span class=\"term\">SUB</span> <span class=\"prod\">registro</span> <span class=\"term\">,</span> <span class=\"prod\">registro</span><br>\n"
        "      <span class=\"prod\">instruccion</span> &rarr; <span class=\"term\">INC</span> <span class=\"prod\">registro</span><br>\n"
        "      <span class=\"prod\">instruccion</span> &rarr; <span class=\"term\">DEC</span> <span class=\"prod\">registro</span><br>\n"
        "      <span class=\"prod\">registro</span> &rarr; <span class=\"term\">AL | AH | BL | BH | CL | CH | DL | DH</span>\n"
        "    </div>\n"
        "  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Resumen del Analisis</div>\n"
        "    <div class=\"stats\">\n"
        "      <div class=\"stat-card stat-ok\"><div class=\"num\">%d</div><div class=\"lbl\">Correctas</div></div>\n"
        "      <div class=\"stat-card stat-err\"><div class=\"num\">%d</div><div class=\"lbl\">Con Error</div></div>\n"
        "      <div class=\"stat-card stat-tot\"><div class=\"num\">%d</div><div class=\"lbl\">Total Lineas</div></div>\n"
        "    </div>\n"
        "  </div>\n"
        , ok, err, numResultadosSint);

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Tabla de Resultados Sintacticos</div>\n"
        "    <table>\n"
        "      <thead>\n"
        "        <tr><th>Linea</th><th>Resultado</th><th>Instruccion</th><th>Descripcion</th></tr>\n"
        "      </thead>\n"
        "      <tbody>\n");

    for (k = 0; k < numResultadosSint; k++) {
        fprintf(f,
            "        <tr><td class=\"linea\">%d</td>"
            "<td><span class=\"badge-%s\">%s</span></td>"
            "<td class=\"instr\">",
            resultadosSint[k].linea,
            resultadosSint[k].correcto ? "ok" : "err",
            resultadosSint[k].correcto ? "OK" : "ERROR");
        escaparHtml(f, resultadosSint[k].instruccion);
        fprintf(f,
            "</td><td class=\"desc-%s\">",
            resultadosSint[k].correcto ? "ok" : "err");
        escaparHtml(f, resultadosSint[k].mensaje);
        fprintf(f, "</td></tr>\n");
    }

    fprintf(f,
        "      </tbody>\n"
        "    </table>\n"
        "  </div>\n"
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Modo Personalizado - Escriba su propio bloque</div>\n"
        "    <p class=\"hint\">Instrucciones validas: ADD reg, reg | SUB reg, reg | INC reg | DEC reg. Cualquier otra sintaxis se marcara como ERROR.</p>\n"
        "    <textarea class=\"textarea-box\" id=\"entradaUsuario\" placeholder=\"Ej: ADD AL, BH\"></textarea>\n"
        "    <div>\n"
        "      <button class=\"btn\" onclick=\"ejecutarSintactico()\">Analizar Sintaxis</button>\n"
        "      <button class=\"btn btn-limpiar\" onclick=\"limpiar()\">Limpiar</button>\n"
        "    </div>\n"
        "    <div class=\"result-area\" id=\"resultadoUsuario\">\n"
        "      <p class=\"hint\">Presione \"Analizar Sintaxis\" para ver los resultados.</p>\n"
        "    </div>\n"
        "  </div>\n"
        "  <footer>Analizador Sintactico 8086 - Generado automaticamente desde C</footer>\n"
        "</div>\n");

    fputs("<script>\n", f);
    fputs(jsLexico, f);
    fputs(
        "function analizarSintactico(tokens) {\n"
        "    const resultados = [];\n"
        "    let k = 0;\n"
        "    while (k < tokens.length) {\n"
        "        const lineaActual = tokens[k].linea;\n"
        "        const sig = [];\n"
        "        while (k < tokens.length && tokens[k].linea === lineaActual) {\n"
        "            if (tokens[k].tipo !== 3 && tokens[k].tipo !== 5) sig.push(tokens[k]);\n"
        "            k++;\n"
        "        }\n"
        "        let instr = '';\n"
        "        for (let j = 0; j < sig.length; j++) {\n"
        "            if (j > 0) instr += ' ';\n"
        "            instr += sig[j].lexema;\n"
        "        }\n"
        "        let correcto = false, mensaje = '';\n"
        "        if (sig.length === 0) {\n"
        "            mensaje = 'Linea vacia - no hay instruccion';\n"
        "        } else if (sig[0].tipo !== 0) {\n"
        "            mensaje = \"Error: se esperaba una INSTRUCCION al inicio, se encontro '\" + sig[0].lexema + \"' (\" + nombreTipoToken[sig[0].tipo] + \")\";\n"
        "        } else if (sig[0].lexema === 'ADD' || sig[0].lexema === 'SUB') {\n"
        "            if (sig.length !== 4) {\n"
        "                mensaje = 'Error: ' + sig[0].lexema + ' requiere 2 operandos - se encontraron ' + sig.length + ' tokens';\n"
        "            } else if (sig[1].tipo !== 1) {\n"
        "                mensaje = \"Error: se esperaba un REGISTRO, se encontro '\" + sig[1].lexema + \"'\";\n"
        "            } else if (sig[2].tipo !== 2) {\n"
        "                mensaje = \"Error: se esperaba una COMA, se encontro '\" + sig[2].lexema + \"'\";\n"
        "            } else if (sig[3].tipo !== 1) {\n"
        "                mensaje = \"Error: se esperaba un REGISTRO, se encontro '\" + sig[3].lexema + \"'\";\n"
        "            } else {\n"
        "                correcto = true;\n"
        "                mensaje = 'Sintaxis correcta: ' + sig[0].lexema + ' ' + sig[1].lexema + ', ' + sig[3].lexema;\n"
        "            }\n"
        "        } else if (sig[0].lexema === 'INC' || sig[0].lexema === 'DEC') {\n"
        "            if (sig.length !== 2) {\n"
        "                mensaje = 'Error: ' + sig[0].lexema + ' requiere 1 operando - se encontraron ' + sig.length + ' tokens';\n"
        "            } else if (sig[1].tipo !== 1) {\n"
        "                mensaje = \"Error: se esperaba un REGISTRO, se encontro '\" + sig[1].lexema + \"'\";\n"
        "            } else {\n"
        "                correcto = true;\n"
        "                mensaje = 'Sintaxis correcta: ' + sig[0].lexema + ' ' + sig[1].lexema;\n"
        "            }\n"
        "        } else {\n"
        "            mensaje = \"Error: instruccion '\" + sig[0].lexema + \"' no reconocida\";\n"
        "        }\n"
        "        resultados.push({linea:lineaActual, correcto:correcto, mensaje:mensaje, instruccion:instr});\n"
        "    }\n"
        "    return resultados;\n"
        "}\n"
        "function ejecutarSintactico() {\n"
        "    const texto = document.getElementById('entradaUsuario').value;\n"
        "    const tokens = analizarLexico(texto);\n"
        "    const resultados = analizarSintactico(tokens);\n"
        "    const cont = document.getElementById('resultadoUsuario');\n"
        "    if (resultados.length === 0) {\n"
        "        cont.innerHTML = '<p class=\"hint\">No se encontraron lineas.</p>';\n"
        "        return;\n"
        "    }\n"
        "    let ok = 0, err = 0;\n"
        "    for (let j = 0; j < resultados.length; j++) {\n"
        "        if (resultados[j].correcto) ok++; else err++;\n"
        "    }\n"
        "    let html = '<div class=\"section\"><div class=\"section-title\">Resumen</div>';\n"
        "    html += '<div class=\"stats\">';\n"
        "    html += '<div class=\"stat-card stat-ok\"><div class=\"num\">' + ok + '</div><div class=\"lbl\">Correctas</div></div>';\n"
        "    html += '<div class=\"stat-card stat-err\"><div class=\"num\">' + err + '</div><div class=\"lbl\">Con Error</div></div>';\n"
        "    html += '<div class=\"stat-card stat-tot\"><div class=\"num\">' + resultados.length + '</div><div class=\"lbl\">Total</div></div>';\n"
        "    html += '</div></div>';\n"
        "    html += '<div class=\"section\"><div class=\"section-title\">Resultados</div>';\n"
        "    html += '<table><thead><tr><th>Linea</th><th>Resultado</th><th>Instruccion</th><th>Descripcion</th></tr></thead><tbody>';\n"
        "    for (let j = 0; j < resultados.length; j++) {\n"
        "        const r = resultados[j];\n"
        "        html += '<tr><td class=\"linea\">' + r.linea + '</td>';\n"
        "        html += '<td><span class=\"badge-' + (r.correcto ? 'ok' : 'err') + '\">' + (r.correcto ? 'OK' : 'ERROR') + '</span></td>';\n"
        "        html += '<td class=\"instr\">' + escaparHtml(r.instruccion) + '</td>';\n"
        "        html += '<td class=\"desc-' + (r.correcto ? 'ok' : 'err') + '\">' + escaparHtml(r.mensaje) + '</td></tr>';\n"
        "    }\n"
        "    html += '</tbody></table></div>';\n"
        "    cont.innerHTML = html;\n"
        "}\n"
        "function limpiar() {\n"
        "    document.getElementById('entradaUsuario').value = '';\n"
        "    document.getElementById('resultadoUsuario').innerHTML = '<p class=\"hint\">Presione \\\"Analizar Sintaxis\\\" para ver los resultados.</p>';\n"
        "}\n"
        "</script>\n"
        "</body>\n"
        "</html>\n", f);

    fclose(f);
    printf("\nArchivo HTML generado: %s\n", archivo);
}

/* ---- Analizador Semantico ---- */

typedef struct {
    int  linea;
    int  nivel;       /* 0=OK, 1=ADVERTENCIA, 2=ERROR */
    char tipo[40];    /* tipo de resultado */
    char mensaje[200];
    char instruccion[128];
} ResultadoSemantico;

static ResultadoSemantico resultadosSem[MAX_TOKENS];

/*
 * Analizador semantico.
 * Recorre los tokens (ya validados sintacticamente) y verifica:
 *
 *  1. Compatibilidad de tipos: ambos operandos de ADD/SUB deben ser
 *     registros de 8 bits (siempre se cumple, pero se reporta).
 *  2. Auto-modificacion: ADD AL, AL (mismo registro como origen y destino).
 *  3. Registro no inicializado: usar un registro como origen antes de
 *     que haya sido escrito (como destino) en alguna instruccion previa.
 *  4. Escritura sin uso posterior: modificar un registro que no se vuelve
 *     a leer en el resto del bloque.
 */

 /*
La función analizarSemantico() recorre las líneas una por una y 
mantiene información de estado de los registros con estas estructuras
 */
static void analizarSemantico(void)
{
    int k = 0;
    int lineaActual;
    Token tokensLinea[16];
    int  nTok;
    int  j;
    char instr[128];

    /* Tabla de seguimiento de registros: 0=no usado, 1=inicializado, 2=leido */
    int registroUsado[8]; /* AL AH BL BH CL CH DL DH */
    int registroEscrito[8]; /* indice de la linea donde se escribio por ultima vez */
    int registroLeido[8];  /* indice de la linea donde se leyo por ultima vez */
    
    for (j = 0; j < 8; j++) {
        registroUsado[j] = 0;
        registroEscrito[j] = -1;
        registroLeido[j] = -1;
    }

    printf("\n");
    printf("====================================================\n");
    printf("  ANALIZADOR SEMANTICO - PROCESADOR 8086\n");
    printf("====================================================\n");
    printf("Linea | Nivel       | Descripcion\n");
    printf("----------------------------------------------------\n");


    /*
 El sintáctico decide si la línea tiene una estructura válida.
El semántico decide si esa estructura tiene sentido.
 */
    while (k < numTokensHtml) {
        lineaActual = tokensHtml[k].linea;

        nTok = 0;
        while (k < numTokensHtml && tokensHtml[k].linea == lineaActual) {
            if (tokensHtml[k].tipo != TOK_ESPACIO &&
                tokensHtml[k].tipo != TOK_FIN) {
                if (nTok < 16) {
                    tokensLinea[nTok++] = tokensHtml[k];
                }
            }
            k++;
        }

        instr[0] = '\0';
        for (j = 0; j < nTok; j++) {
            if (j > 0) strcat(instr, " ");
            strcat(instr, tokensLinea[j].lexema);
        }

        if (numResultadosSem < MAX_TOKENS) {
            resultadosSem[numResultadosSem].linea = lineaActual;
            strcpy(resultadosSem[numResultadosSem].instruccion, instr);

            if (nTok == 0) {
                resultadosSem[numResultadosSem].nivel = 2;
                strcpy(resultadosSem[numResultadosSem].tipo, "ERROR");
                snprintf(resultadosSem[numResultadosSem].mensaje,
                         sizeof(resultadosSem[numResultadosSem].mensaje),
                         "Linea vacia");
            }
            else if (tokensLinea[0].tipo != TOK_INSTRUCCION) {
                resultadosSem[numResultadosSem].nivel = 2;
                strcpy(resultadosSem[numResultadosSem].tipo, "ERROR");
                snprintf(resultadosSem[numResultadosSem].mensaje,
                         sizeof(resultadosSem[numResultadosSem].mensaje),
                         "No es una instruccion valida");
            }
            else if (strcmp(tokensLinea[0].lexema, "ADD") == 0 ||
                     strcmp(tokensLinea[0].lexema, "SUB") == 0) {

                if (nTok != 4 || tokensLinea[1].tipo != TOK_REGISTRO ||
                    tokensLinea[2].tipo != TOK_COMA ||
                    tokensLinea[3].tipo != TOK_REGISTRO) {
                    resultadosSem[numResultadosSem].nivel = 2;
                    strcpy(resultadosSem[numResultadosSem].tipo, "ERROR");
                    snprintf(resultadosSem[numResultadosSem].mensaje,
                             sizeof(resultadosSem[numResultadosSem].mensaje),
                             "Error sintactico - no se puede analizar semanticamente");
                }
                else {
                    int rDest = indiceReg(tokensLinea[1].lexema);
                    int rSrc  = indiceReg(tokensLinea[3].lexema);
                    int hayAdvertencia = 0;
                    char msg[200];
                    msg[0] = '\0';

                    /* 1. Compatibilidad de tipos */
                    strcat(msg, "Tipos compatibles: ambos operandos son registros de 8 bits");

                    /* 2. Registro no inicializado */
                    if (registroUsado[rSrc] == 0) {
                        strcat(msg, ". ADVERTENCIA: el registro origen ");
                        strcat(msg, tokensLinea[3].lexema);
                        strcat(msg, " no ha sido inicializado");
                        hayAdvertencia = 1;
                    }

                    /* 3. Auto-modificacion */
                    if (rDest == rSrc) {
                        strcat(msg, ". ADVERTENCIA: auto-modificacion (");
                        strcat(msg, tokensLinea[1].lexema);
                        strcat(msg, " se usa como origen y destino)");
                        hayAdvertencia = 1;
                    }

                    /* Marcar registros */
                    registroUsado[rDest] = 1;
                    registroUsado[rSrc] = 1;
                    registroEscrito[rDest] = lineaActual;
                    registroLeido[rSrc] = lineaActual;

                    if (hayAdvertencia) {
                        resultadosSem[numResultadosSem].nivel = 1;
                        strcpy(resultadosSem[numResultadosSem].tipo, "ADVERTENCIA");
                    } else {
                        resultadosSem[numResultadosSem].nivel = 0;
                        strcpy(resultadosSem[numResultadosSem].tipo, "OK");
                    }
                    strncpy(resultadosSem[numResultadosSem].mensaje, msg,
                            sizeof(resultadosSem[numResultadosSem].mensaje) - 1);
                    resultadosSem[numResultadosSem].mensaje[sizeof(resultadosSem[numResultadosSem].mensaje) - 1] = '\0';
                }
            }
            else if (strcmp(tokensLinea[0].lexema, "INC") == 0 ||
                     strcmp(tokensLinea[0].lexema, "DEC") == 0) {

                if (nTok != 2 || tokensLinea[1].tipo != TOK_REGISTRO) {
                    resultadosSem[numResultadosSem].nivel = 2;
                    strcpy(resultadosSem[numResultadosSem].tipo, "ERROR");
                    snprintf(resultadosSem[numResultadosSem].mensaje,
                             sizeof(resultadosSem[numResultadosSem].mensaje),
                             "Error sintactico - no se puede analizar semanticamente");
                }
                else {
                    int rDest = indiceReg(tokensLinea[1].lexema);
                    char msg[200];
                    msg[0] = '\0';

                    strcat(msg, "Tipos compatibles: operando es registro de 8 bits");

                    /* INC/DEC solo modifica, no lee otro registro */
                    if (registroUsado[rDest] == 0) {
                        strcat(msg, ". NOTA: el registro ");
                        strcat(msg, tokensLinea[1].lexema);
                        strcat(msg, " se modifica por primera vez (valor previo desconocido)");
                    }

                    registroUsado[rDest] = 1;
                    registroEscrito[rDest] = lineaActual;

                    resultadosSem[numResultadosSem].nivel = 0;
                    strcpy(resultadosSem[numResultadosSem].tipo, "OK");
                    strncpy(resultadosSem[numResultadosSem].mensaje, msg,
                            sizeof(resultadosSem[numResultadosSem].mensaje) - 1);
                    resultadosSem[numResultadosSem].mensaje[sizeof(resultadosSem[numResultadosSem].mensaje) - 1] = '\0';
                }
            }
            else {
                resultadosSem[numResultadosSem].nivel = 2;
                strcpy(resultadosSem[numResultadosSem].tipo, "ERROR");
                snprintf(resultadosSem[numResultadosSem].mensaje,
                         sizeof(resultadosSem[numResultadosSem].mensaje),
                         "Instruccion no reconocida");
            }

            printf(" %3d  | %-11s | %s\n",
                   resultadosSem[numResultadosSem].linea,
                   resultadosSem[numResultadosSem].tipo,
                   resultadosSem[numResultadosSem].mensaje);

            numResultadosSem++;
        }
    }

    /* 4. Deteccion de escrituras sin uso posterior */
    printf("----------------------------------------------------\n");
    {
        int ok = 0, adv = 0, err = 0;
        for (j = 0; j < numResultadosSem; j++) {
            if (resultadosSem[j].nivel == 0) ok++;
            else if (resultadosSem[j].nivel == 1) adv++;
            else err++;
        }
        printf("Resumen: %d correctas, %d advertencias, %d errores, %d total\n",
               ok, adv, err, numResultadosSem);
    }

    /* Reportar registros escritos pero no leidos posteriormente */
    {
        int r;
        for (r = 0; r < 8; r++) {
            if (registroEscrito[r] != -1 && registroLeido[r] < registroEscrito[r]) {
                printf(" NOTA: registro %s fue escrito (linea %d) pero no se leio despues\n",
                       registros8[r], registroEscrito[r]);
            }
        }
    }
    printf("====================================================\n");
    printf("Fin del analisis semantico.\n");
}

/*
 * Genera el HTML del analizador semantico.
 */
static void generarHtmlSemantico(const char *entrada, const char *archivo,
                                 const char *titulo)
{
    FILE *f = fopen(archivo, "w");
    int  k;
    int  ok = 0, adv = 0, err = 0;

    if (!f) {
        fprintf(stderr, "Error: no se pudo crear %s\n", archivo);
        return;
    }

    for (k = 0; k < numResultadosSem; k++) {
        if (resultadosSem[k].nivel == 0) ok++;
        else if (resultadosSem[k].nivel == 1) adv++;
        else err++;
    }

    fprintf(f,
        "<!DOCTYPE html>\n"
        "<html lang=\"es\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "<title>Analizador Semantico - Procesador 8086</title>\n"
        "<style>\n"
        "  header h1 {\n"
        "    background: linear-gradient(135deg, #f59e0b, #f97316);\n"
        "    -webkit-background-clip: text; -webkit-text-fill-color: transparent;\n"
        "    background-clip: text;\n"
        "  }\n"
        "  .section-title { color: #f59e0b; }\n"
        "  .stat-ok  .num { color: #34d399; }\n"
        "  .stat-adv .num { color: #fbbf24; }\n"
        "  .stat-err .num { color: #f87171; }\n"
        "  .stat-tot .num { color: #c084fc; }\n"
        "  .badge-ok  {\n"
        "    display: inline-block; padding: 0.2rem 0.6rem;\n"
        "    border-radius: 4px; font-size: 0.8rem; font-weight: 600;\n"
        "    background: rgba(52,211,153,0.15); color: #34d399;\n"
        "    border: 1px solid rgba(52,211,153,0.3);\n"
        "  }\n"
        "  .badge-adv {\n"
        "    display: inline-block; padding: 0.2rem 0.6rem;\n"
        "    border-radius: 4px; font-size: 0.8rem; font-weight: 600;\n"
        "    background: rgba(251,191,36,0.15); color: #fbbf24;\n"
        "    border: 1px solid rgba(251,191,36,0.3);\n"
        "  }\n"
        "  .badge-err {\n"
        "    display: inline-block; padding: 0.2rem 0.6rem;\n"
        "    border-radius: 4px; font-size: 0.8rem; font-weight: 600;\n"
        "    background: rgba(248,113,113,0.15); color: #f87171;\n"
        "    border: 1px solid rgba(248,113,113,0.3);\n"
        "  }\n"
        "  .instr { font-family: 'Courier New', monospace; color: #cbd5e1; }\n"
        "  .desc-ok  { color: #94a3b8; }\n"
        "  .desc-adv { color: #fbbf24; }\n"
        "  .desc-err { color: #f87171; }\n"
        "  .grammar-box .prod { color: #f59e0b; }\n"
        "  .btn { background: linear-gradient(135deg, #f59e0b, #f97316); }\n"
        "  .textarea-box:focus { border-color: #f59e0b; }\n"
        "  .nav-link.sem { background: #f59e0b; color: #fff; border-color: #f59e0b; }\n"
        "%s"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<div class=\"container\">\n"
        "  <header>\n"
        "    <h1>Analizador Semantico - Procesador 8086</h1>\n"
        "    <p>%s</p>\n"
        "  </header>\n"
        , cssBase, titulo);

    fprintf(f,
        "  <div class=\"nav-links\">\n"
        "    <a class=\"nav-link\" href=\"resultado_lexico.html\">Lexico</a>\n"
        "    <a class=\"nav-link\" href=\"resultado_sintactico.html\">Sintactico</a>\n"
        "    <a class=\"nav-link sem\" href=\"#\">Semantico</a>\n"
        "  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Codigo de Entrada</div>\n"
        "    <div class=\"input-box\">");
    escaparHtml(f, entrada);
    fprintf(f, "</div>\n  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Reglas Semanticas</div>\n"
        "    <div class=\"grammar-box\">\n"
        "      <span class=\"prod\">1. Compatibilidad de tipos:</span> ambos operandos de ADD/SUB deben ser registros de 8 bits<br>\n"
        "      <span class=\"prod\">2. Auto-modificacion:</span> se advierte si el registro origen y destino son el mismo (ej: ADD AL, AL)<br>\n"
        "      <span class=\"prod\">3. Registro no inicializado:</span> se advierte si un registro se lee antes de ser escrito<br>\n"
        "      <span class=\"prod\">4. Escritura sin uso:</span> se reporta si un registro se modifica pero no se vuelve a leer<br>\n"
        "      <span class=\"prod\">5. Seguimiento de registros:</span> AL, AH, BL, BH, CL, CH, DL, DH <span class=\"term\">(8 bits)</span>\n"
        "    </div>\n"
        "  </div>\n");

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Resumen del Analisis</div>\n"
        "    <div class=\"stats\">\n"
        "      <div class=\"stat-card stat-ok\"><div class=\"num\">%d</div><div class=\"lbl\">Correctas</div></div>\n"
        "      <div class=\"stat-card stat-adv\"><div class=\"num\">%d</div><div class=\"lbl\">Advertencias</div></div>\n"
        "      <div class=\"stat-card stat-err\"><div class=\"num\">%d</div><div class=\"lbl\">Errores</div></div>\n"
        "      <div class=\"stat-card stat-tot\"><div class=\"num\">%d</div><div class=\"lbl\">Total Lineas</div></div>\n"
        "    </div>\n"
        "  </div>\n"
        , ok, adv, err, numResultadosSem);

    fprintf(f,
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Tabla de Resultados Semanticos</div>\n"
        "    <table>\n"
        "      <thead>\n"
        "        <tr><th>Linea</th><th>Nivel</th><th>Instruccion</th><th>Descripcion</th></tr>\n"
        "      </thead>\n"
        "      <tbody>\n");

    for (k = 0; k < numResultadosSem; k++) {
        const char *clase = resultadosSem[k].nivel == 0 ? "ok" :
                            resultadosSem[k].nivel == 1 ? "adv" : "err";
        fprintf(f,
            "        <tr><td class=\"linea\">%d</td>"
            "<td><span class=\"badge-%s\">%s</span></td>"
            "<td class=\"instr\">",
            resultadosSem[k].linea,
            clase,
            resultadosSem[k].tipo);
        escaparHtml(f, resultadosSem[k].instruccion);
        fprintf(f,
            "</td><td class=\"desc-%s\">",
            clase);
        escaparHtml(f, resultadosSem[k].mensaje);
        fprintf(f, "</td></tr>\n");
    }

    fprintf(f,
        "      </tbody>\n"
        "    </table>\n"
        "  </div>\n"
        "  <div class=\"section\">\n"
        "    <div class=\"section-title\">Modo Personalizado - Escriba su propio bloque</div>\n"
        "    <p class=\"hint\">Escriba instrucciones validas (ADD, SUB, INC, DEC con registros de 8 bits) para ver el analisis semantico.</p>\n"
        "    <textarea class=\"textarea-box\" id=\"entradaUsuario\" placeholder=\"Ej: ADD AL, BH\"></textarea>\n"
        "    <div>\n"
        "      <button class=\"btn\" onclick=\"ejecutarSemantico()\">Analizar Semantica</button>\n"
        "      <button class=\"btn btn-limpiar\" onclick=\"limpiar()\">Limpiar</button>\n"
        "    </div>\n"
        "    <div class=\"result-area\" id=\"resultadoUsuario\">\n"
        "      <p class=\"hint\">Presione \"Analizar Semantica\" para ver los resultados.</p>\n"
        "    </div>\n"
        "  </div>\n"
        "  <footer>Analizador Semantico 8086 - Generado automaticamente desde C</footer>\n"
        "</div>\n");

    fputs("<script>\n", f);
    fputs(jsLexico, f);
    fputs(
        "function analizarSintactico(tokens) {\n"
        "    const resultados = [];\n"
        "    let k = 0;\n"
        "    while (k < tokens.length) {\n"
        "        const lineaActual = tokens[k].linea;\n"
        "        const sig = [];\n"
        "        while (k < tokens.length && tokens[k].linea === lineaActual) {\n"
        "            if (tokens[k].tipo !== 3 && tokens[k].tipo !== 5) sig.push(tokens[k]);\n"
        "            k++;\n"
        "        }\n"
        "        let instr = '';\n"
        "        for (let j = 0; j < sig.length; j++) {\n"
        "            if (j > 0) instr += ' ';\n"
        "            instr += sig[j].lexema;\n"
        "        }\n"
        "        let correcto = false, mensaje = '';\n"
        "        if (sig.length === 0) { mensaje = 'Linea vacia'; }\n"
        "        else if (sig[0].tipo !== 0) { mensaje = 'No es instruccion'; }\n"
        "        else if (sig[0].lexema === 'ADD' || sig[0].lexema === 'SUB') {\n"
        "            if (sig.length !== 4) { mensaje = 'Error sintactico'; }\n"
        "            else if (sig[1].tipo !== 1 || sig[2].tipo !== 2 || sig[3].tipo !== 1) { mensaje = 'Error sintactico'; }\n"
        "            else { correcto = true; }\n"
        "        } else if (sig[0].lexema === 'INC' || sig[0].lexema === 'DEC') {\n"
        "            if (sig.length !== 2 || sig[1].tipo !== 1) { mensaje = 'Error sintactico'; }\n"
        "            else { correcto = true; }\n"
        "        } else { mensaje = 'Instruccion no reconocida'; }\n"
        "        resultados.push({linea:lineaActual, correcto:correcto, mensaje:mensaje, instruccion:instr, tokens:sig});\n"
        "    }\n"
        "    return resultados;\n"
        "}\n"
        "\n"
        "function analizarSemantico(tokens) {\n"
        "    const sintactico = analizarSintactico(tokens);\n"
        "    const resultados = [];\n"
        "    const regUsado = {};\n"
        "    const regEscrito = {};\n"
        "    const regLeido = {};\n"
        "    for (let j = 0; j < sintactico.length; j++) {\n"
        "        const s = sintactico[j];\n"
        "        if (!s.correcto) {\n"
        "            resultados.push({linea:s.linea, nivel:2, tipo:'ERROR', mensaje:'Error sintactico - no se puede analizar', instruccion:s.instruccion});\n"
        "            continue;\n"
        "        }\n"
        "        const sig = s.tokens;\n"
        "        let nivel = 0, tipo = 'OK', msg = '';\n"
        "        if (sig[0].lexema === 'ADD' || sig[0].lexema === 'SUB') {\n"
        "            const rDest = sig[1].lexema;\n"
        "            const rSrc = sig[3].lexema;\n"
        "            msg = 'Tipos compatibles: ambos operandos son registros de 8 bits';\n"
        "            if (!regUsado[rSrc]) {\n"
        "                msg += '. ADVERTENCIA: el registro origen ' + rSrc + ' no ha sido inicializado';\n"
        "                nivel = 1; tipo = 'ADVERTENCIA';\n"
        "            }\n"
        "            if (rDest === rSrc) {\n"
        "                msg += '. ADVERTENCIA: auto-modificacion (' + rDest + ' como origen y destino)';\n"
        "                nivel = 1; tipo = 'ADVERTENCIA';\n"
        "            }\n"
        "            regUsado[rDest] = true; regUsado[rSrc] = true;\n"
        "            regEscrito[rDest] = s.linea; regLeido[rSrc] = s.linea;\n"
        "        } else if (sig[0].lexema === 'INC' || sig[0].lexema === 'DEC') {\n"
        "            const rDest = sig[1].lexema;\n"
        "            msg = 'Tipos compatibles: operando es registro de 8 bits';\n"
        "            if (!regUsado[rDest]) {\n"
        "                msg += '. NOTA: el registro ' + rDest + ' se modifica por primera vez (valor previo desconocido)';\n"
        "            }\n"
        "            regUsado[rDest] = true;\n"
        "            regEscrito[rDest] = s.linea;\n"
        "        }\n"
        "        resultados.push({linea:s.linea, nivel:nivel, tipo:tipo, mensaje:msg, instruccion:s.instruccion});\n"
        "    }\n"
        "    return resultados;\n"
        "}\n"
        "\n"
        "function ejecutarSemantico() {\n"
        "    const texto = document.getElementById('entradaUsuario').value;\n"
        "    const tokens = analizarLexico(texto);\n"
        "    const resultados = analizarSemantico(tokens);\n"
        "    const cont = document.getElementById('resultadoUsuario');\n"
        "    if (resultados.length === 0) {\n"
        "        cont.innerHTML = '<p class=\"hint\">No se encontraron lineas.</p>';\n"
        "        return;\n"
        "    }\n"
        "    let ok = 0, adv = 0, err = 0;\n"
        "    for (let j = 0; j < resultados.length; j++) {\n"
        "        if (resultados[j].nivel === 0) ok++;\n"
        "        else if (resultados[j].nivel === 1) adv++;\n"
        "        else err++;\n"
        "    }\n"
        "    let html = '<div class=\"section\"><div class=\"section-title\">Resumen</div>';\n"
        "    html += '<div class=\"stats\">';\n"
        "    html += '<div class=\"stat-card stat-ok\"><div class=\"num\">' + ok + '</div><div class=\"lbl\">Correctas</div></div>';\n"
        "    html += '<div class=\"stat-card stat-adv\"><div class=\"num\">' + adv + '</div><div class=\"lbl\">Advertencias</div></div>';\n"
        "    html += '<div class=\"stat-card stat-err\"><div class=\"num\">' + err + '</div><div class=\"lbl\">Errores</div></div>';\n"
        "    html += '<div class=\"stat-card stat-tot\"><div class=\"num\">' + resultados.length + '</div><div class=\"lbl\">Total</div></div>';\n"
        "    html += '</div></div>';\n"
        "    html += '<div class=\"section\"><div class=\"section-title\">Resultados Semanticos</div>';\n"
        "    html += '<table><thead><tr><th>Linea</th><th>Nivel</th><th>Instruccion</th><th>Descripcion</th></tr></thead><tbody>';\n"
        "    for (let j = 0; j < resultados.length; j++) {\n"
        "        const r = resultados[j];\n"
        "        const cls = r.nivel === 0 ? 'ok' : r.nivel === 1 ? 'adv' : 'err';\n"
        "        html += '<tr><td class=\"linea\">' + r.linea + '</td>';\n"
        "        html += '<td><span class=\"badge-' + cls + '\">' + r.tipo + '</span></td>';\n"
        "        html += '<td class=\"instr\">' + escaparHtml(r.instruccion) + '</td>';\n"
        "        html += '<td class=\"desc-' + cls + '\">' + escaparHtml(r.mensaje) + '</td></tr>';\n"
        "    }\n"
        "    html += '</tbody></table></div>';\n"
        "    cont.innerHTML = html;\n"
        "}\n"
        "function limpiar() {\n"
        "    document.getElementById('entradaUsuario').value = '';\n"
        "    document.getElementById('resultadoUsuario').innerHTML = '<p class=\"hint\">Presione \\\"Analizar Semantica\\\" para ver los resultados.</p>';\n"
        "}\n"
        "</script>\n"
        "</body>\n"
        "</html>\n", f);

    fclose(f);
    printf("\nArchivo HTML generado: %s\n", archivo);
}

/*
 * Lee un bloque de entrada multilinea desde teclado.
 */
static void leerEntradaUsuario(char *buffer, int tam)
{
    char linea[512];
    int  pos = 0;

    printf("Escriba su bloque de entrada (escriba FIN en una linea para terminar):\n");

    while (fgets(linea, sizeof(linea), stdin)) {
        int len = (int)strlen(linea);
        if (len > 0 && linea[len - 1] == '\n') {
            linea[len - 1] = '\0';
            len--;
        }

        if (len == 3) {
            char tmp[4];
            int j;
            for (j = 0; j < 3; j++)
                tmp[j] = (char)toupper((unsigned char)linea[j]);
            tmp[3] = '\0';
            if (strcmp(tmp, "FIN") == 0)
                break;
        }

        if (pos + len + 2 < tam) {
            strcpy(buffer + pos, linea);
            pos += len;
            buffer[pos++] = '\n';
            buffer[pos] = '\0';
        } else {
            printf("(Buffer lleno, se corta la entrada)\n");
            break;
        }
    }

    if (pos == 0) {
        strcpy(buffer, "");
    }
}

/*
 * Programa principal.
 * Menu con cuatro opciones:
 *   1) Analizador Lexico
 *   2) Analizador Sintactico
 *   3) Analizador Semantico
 *   4) Salir
 */
int main(void)
{
    const char *entradaEstatica =
        "ADD AL, BH\n"
        "SUB AH, BL\n"
        "INC AL \n"
        "DEC BH\n"
        "ADD BL, CH\n"
        "SUB CH, DL \n"
        "INC DL\n"
        "DEC AL\n"
        "ADD AH, BL\n"
        "SUB BL, AL\n";

    char  entradaUsuario[MAX_INPUT];
    int   opcion, subopcion;

    printf("========================================\n");
    printf("  ANALIZADOR - PROCESADOR 8086\n");
    printf("========================================\n");
    printf("  1) Analizador Lexico\n");
    printf("  2) Analizador Sintactico\n");
    printf("  3) Analizador Semantico\n");
    printf("  4) Salir\n");
    printf("----------------------------------------\n");
    printf("Seleccione una opcion: ");

    if (scanf("%d", &opcion) != 1) {
        printf("Entrada invalida. Saliendo.\n");
        return 1;
    }

    while (getchar() != '\n')
        ;

    switch (opcion) {
    case 1:
        printf("\n  --- Analizador Lexico ---\n");
        printf("  1) Bloque estatico predefinido\n");
        printf("  2) Ingresar mi propio bloque\n");
        printf("  Seleccione: ");
        if (scanf("%d", &subopcion) != 1) { printf("Entrada invalida.\n"); return 1; }
        while (getchar() != '\n') ;

        if (subopcion == 1) {
            resetTokens();
            analizar(entradaEstatica);
            generarHtml(entradaEstatica, "resultado_lexico.html",
                        "Bloque estatico predefinido");
        } else if (subopcion == 2) {
            leerEntradaUsuario(entradaUsuario, MAX_INPUT);
            if (strlen(entradaUsuario) == 0) {
                printf("No se ingreso ningun texto. Saliendo.\n");
                return 0;
            }
            resetTokens();
            analizar(entradaUsuario);
            generarHtml(entradaUsuario, "resultado_lexico.html",
                        "Bloque ingresado por el usuario");
        } else {
            printf("Subopcion no valida.\n");
        }
        break;

    case 2:
        printf("\n  --- Analizador Sintactico ---\n");
        printf("  1) Bloque estatico predefinido\n");
        printf("  2) Ingresar mi propio bloque\n");
        printf("  Seleccione: ");
        if (scanf("%d", &subopcion) != 1) { printf("Entrada invalida.\n"); return 1; }
        while (getchar() != '\n') ;

        if (subopcion == 1) {
            resetTokens();
            analizar(entradaEstatica);
            analizarSintactico();
            generarHtmlSintactico(entradaEstatica, "resultado_sintactico.html",
                                  "Bloque estatico predefinido");
        } else if (subopcion == 2) {
            leerEntradaUsuario(entradaUsuario, MAX_INPUT);
            if (strlen(entradaUsuario) == 0) {
                printf("No se ingreso ningun texto. Saliendo.\n");
                return 0;
            }
            resetTokens();
            analizar(entradaUsuario);
            analizarSintactico();
            generarHtmlSintactico(entradaUsuario, "resultado_sintactico.html",
                                  "Bloque ingresado por el usuario");
        } else {
            printf("Subopcion no valida.\n");
        }
        break;

    case 3:
        printf("\n  --- Analizador Semantico ---\n");
        printf("  1) Bloque estatico predefinido\n");
        printf("  2) Ingresar mi propio bloque\n");
        printf("  Seleccione: ");
        if (scanf("%d", &subopcion) != 1) { printf("Entrada invalida.\n"); return 1; }
        while (getchar() != '\n') ;

        if (subopcion == 1) {
            resetTokens();
            analizar(entradaEstatica);
            analizarSintactico();
            analizarSemantico();
            generarHtmlSemantico(entradaEstatica, "resultado_semantico.html",
                                "Bloque estatico predefinido");
        } else if (subopcion == 2) {
            leerEntradaUsuario(entradaUsuario, MAX_INPUT);
            if (strlen(entradaUsuario) == 0) {
                printf("No se ingreso ningun texto. Saliendo.\n");
                return 0;
            }
            resetTokens();
            analizar(entradaUsuario);
            analizarSintactico();
            analizarSemantico();
            generarHtmlSemantico(entradaUsuario, "resultado_semantico.html",
                                "Bloque ingresado por el usuario");
        } else {
            printf("Subopcion no valida.\n");
        }
        break;

    case 4:
        printf("Hasta luego.\n");
        break;

    default:
        printf("Opcion no valida. Saliendo.\n");
        break;
    }

    return 0;
}
