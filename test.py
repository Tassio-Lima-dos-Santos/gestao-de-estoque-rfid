'''

ESTE CÓDIGO SERVE PARA TESTE DA INSERÇÃO E EXTRAÇÃO DE VALORES 
DO BANCO DE DADOS, ATUALIZANDO O VALOR DE "tag" '''

import serial # importa a biblioteca para comunicação serial (arduino)
import mysql.connector # importa a biblioteca para conectar o MySQL

# configuração da porta serial (Serial.begin(9600)) e entrada USB
# Linux ⟶ [sudo dmesg | grep tty] para verificação da porta utilizada
# timeout = tempo limite de espera

'''
    Testes
'''

tag = "y6758u4cv927g" # novo dado

'''
    Fim dos Testes
'''

# conectar ao banco de dados
conn = mysql.connector.connect(
    host="localhost",
    user="adrianfelsky",
    password="dan!eLeo9/01",
    database="tecidos"
)

# cria o cursor ⟶ enviar comandos SQL
cursor = conn.cursor()

# verifica se a tag rfid está contida no banco
def tagExistente(cursor,tag):
    # executa o comando sql que extrai dados no banco
    cursor.execute("""
                   SELECT nome, fornecedor, lote FROM fornecedores WHERE rfid = %s
                   """, (tag,))
    resultado = cursor.fetchone() # retorna um tuple da linha selecionada ⟶ resultado = (nome, fornecedor, lote)   ;  resultado = ([0],[1],[2])
    return resultado  # NULL se não encontrado

try:    # capturar erros
    if tag: # evita NULL
        print(f"Tag detectada: {tag}") # print rfid

        dados = tagExistente(cursor, tag) # verifica se a tag já existe no banco

        # se != NULL == tag já está no banco
        # informa os dados da tag cadastrada
        if dados: 
            print("Tag já cadastrada:\n")
            print(f"  Nome: {dados[0]}")
            print(f"  Fornecedor: {dados[1]}")
            print(f"  Lote: {dados[2]}")
            print("\nFim da leitura.")                

        else: # cadastro da tag
            print("Tag ainda não cadastrada.")
            # solicita a inclusão na base
            opcao = input("Deseja cadastrar essa tag no banco? (s/n): ").strip().lower() 

            if opcao == 's':
                # solicita os dados via terminal
                nome = input("Nome do tecido: ")
                fornecedor = input("Fornecedor: ")
                lote = input("Lote: ")

                try: # evita duplicidade de dados: rfid ⟶ UNIQUE
                    # executa o comando sql que insere dados no banco
                    cursor.execute("""
                        INSERT INTO fornecedores (rfid, nome, fornecedor, lote)
                        VALUES (%s, %s, %s, %s)
                    """, (tag, nome, fornecedor, lote))
                    conn.commit() # confirma a atividade no banco
                    print("Registro salvo!\n")

                # em caso de erro desfaz a operação
                except mysql.connector.Error as err:
                    print(f"Erro no banco: {err}")
                    conn.rollback() # desfaz a operação

            else:
                print("Encerrando sem inserir dados.")

except KeyboardInterrupt:
    print("\n Encerrando programa...") # desiste da operação em caso 'ctrl+c'

finally: #fecha tudo
    cursor.close()
    conn.close()
    #porta_serial.close()
    print("Conexões fechadas.")
