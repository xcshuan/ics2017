#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_PLUS, TK_MINU, TK_MULT, TK_DIVI, TK_DEC, TK_LEB, TK_RIB,TK_REG,TK_HEX, TK_NEQ, TK_LAND, TK_LOR, TK_NE,TK_DEFE
	  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\(", TK_LEB},     //Left brace
  {"\\)", TK_RIB},		//Right brace
  {"\\+", TK_PLUS},    // plus
  {"-", TK_MINU},		//Minus 
  {"\\*", TK_MULT},			//multiplication sign
  {"/", TK_DIVI},			//division sign
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},		//Not Equal
  {"&&", TK_LAND},
  {"\\|\\|", TK_LOR},
  {"^0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_DEC},      //digital 
  {"\\$e[a-z]+",TK_REG},	//reg
  {"!", TK_NE}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
}
}
}
typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

void check_sign(uint32_t p, uint32_t q){
	if(tokens[p].type == TK_MULT) tokens[p].type = TK_DEFE;
	for(int i = p + 1; i <= q; i++){
		if(tokens[i].type == TK_MULT && tokens[i - 1].type != TK_RIB && tokens[i - 1].type != TK_DEC &&  tokens[i - 1].type != TK_HEX &&  tokens[i - 1].type != TK_REG){
			tokens[i].type = TK_DEFE;
		}
	}
}

bool check_parentheses(uint32_t p, uint32_t q, bool *success ){
	uint32_t flag = 0;
	uint32_t n = 0;
	if(tokens[p].type != TK_LEB) n++;
	for(int i = p; i <= q; i++){
		if(tokens[i].type == TK_LEB){
			flag++;
		}
		else if(tokens[i].type == TK_RIB){
			flag--;
		}
		if(flag == 0 && i != p && i != q) n++;
		if(flag < 0) {
			*success = false;
			n++;
		}
	} 
	if(flag != 0) {
		*success = false;
		n++;
	}

	if(n > 0) return false;
	return true;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		
		
		if(rules[i].token_type == TK_NOTYPE) break;	
		
		tokens[nr_token].type = rules[i].token_type;
        
		switch (rules[i].token_type) {
			case TK_EQ:
			case TK_NEQ:
			case TK_LAND:
			case TK_LOR:
			case TK_PLUS: 
			case TK_MINU: 
			case TK_MULT: 
			case TK_DIVI: 
			case TK_LEB: 
			case TK_NE:
			case TK_RIB:break;
			case TK_HEX:
			case TK_REG:
			case TK_DEC:tokens[nr_token].str[0] = '\0';
						strncat(tokens[nr_token].str, substr_start,(substr_len < 32? substr_len:32));
        }
		nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
 // bool success;
  /*check_parentheses(0, nr_token - 1, &success);
  if(success == false){
	  return false;
  }*/
  check_sign(0, nr_token - 1);
  return true;
}



uint32_t priority(uint32_t type){			//return the priority of token
	if(type == TK_LEB || type == TK_RIB)
		return 1;
	else if(type == TK_NE ||type == TK_DEFE)
		return 2;
	else if(type == TK_MULT || type == TK_DIVI)
		return 3;
	else if(type == TK_PLUS || type == TK_MINU)
		return 4;
	else if(type == TK_NEQ || type == TK_EQ)
		return 7;
	else return 0;

}

uint32_t find_op(uint32_t p, uint32_t q){  //find dominate oprator
	uint32_t x = p;
	for(int i = p;i <= q;i++){
		if(tokens[i].type == TK_LEB){
			while(1){
				i++;
				if(tokens[i].type == TK_RIB) break;
			}
		}
		if((priority(tokens[i].type) >= priority(tokens[x].type)) && i > x){
			x = i;
		}
	}

	return x;
}

uint32_t eval(uint32_t p,uint32_t q){	//evaluate
	bool success;
	if(p > q){
		/*Bad expression*/
		return -1;
	}
	else if (p == q){
		/*Single token.
		 * For  now this token should be a number.
		 * return the value of the number*/
		uint32_t n;
		if(tokens[p].type == TK_DEC){
			assert(sscanf(tokens[p].str,"%d",&n));
			return n;
		}
		else if(tokens[p].type == TK_HEX){
			assert(sscanf(tokens[p].str + 2, "%x",&n));
					return n;
				}
		else if(tokens[p].type == TK_REG){
		if (strcmp(tokens[p].str, "$eax") == 0) return cpu.eax;
		else if (strcmp(tokens[p].str, "$ebx") == 0)  return cpu.ebx;
		else if (strcmp(tokens[p].str, "$ecx") == 0)  return cpu.ecx;
	    else if (strcmp(tokens[p].str, "$edx") == 0)  return cpu.edx;
	    else if (strcmp(tokens[p].str, "$ebp") == 0)  return cpu.ebp;
	    else if (strcmp(tokens[p].str, "$esp") == 0)  return cpu.esp;
	    else if (strcmp(tokens[p].str, "$esi") == 0)  return cpu.esi;
	    else if (strcmp(tokens[p].str, "$edi") == 0)  return cpu.edi;
	    else if (strcmp(tokens[p].str, "$eip") == 0)  return cpu.eip;
		else printf("REG Intput Error!");
		}
	}
	else if(check_parentheses(p,q,&success) == true){
		/*The expression is surrounded by a matched pair of parentheses.
		 * If that is the case, just throw away the parentheses.
		 */
		return eval(p + 1, q - 1);
	}
	else{
		uint32_t op = find_op(p, q);
		uint32_t val1 = eval(p, op - 1);
		uint32_t val2 = eval(op + 1, q);

		switch(tokens[op].type){
			case TK_PLUS: return val1 + val2;
			case TK_MINU: return val1 - val2;
			case TK_MULT: return val1 * val2;
			case TK_DIVI: return val1 / val2;
			case TK_NE: return !(val2);
			case TK_DEFE: return vaddr_read(val2, 4);
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case TK_LAND: return val1 && val2;
			case TK_LOR: return val1 || val2;
			default:assert(0);
		}
		/*Todo*/
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
 uint32_t a = (eval(0, nr_token-1));
 *success = true;
 return a;
 
}
