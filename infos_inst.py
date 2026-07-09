import os 
import shutil

def move_inst(path:str): 
	"""
	Cette fonction bouge les instances a 
	n = 100, 500 et 1000 dans un dossier "small_inst"	
	"""

	folder_content = os.listdir(path)
	for inst in folder_content: 
		parsed_inst = inst.split("_")

		n =  int(parsed_inst[0])
		
		if n == 100 or n == 500 or n == 1000:
			
			source = os.path.join("inst", inst)
			destination = os.path.join("small_inst",inst)
			
			try:
				shutil.copy2(source, destination)
				print("copie fichier ", source, " reussie")
			except FileNotFoundError:
				print("erreur, le fichier n'existe pas")




def get_inst_folder_infos(path:str):

	folder_content = os.listdir(path)
	counter = dict() # dictionnaire pr contenir les informations

	for inst in folder_content:

		parsed_inst = inst.split("_") # séparer par "_"
		
		n = int(parsed_inst[0]) # n tj a la position 0
		k = int(parsed_inst[2]) # k tj a la position 2

		if n in counter.keys(): # si n est déjà une clé existente 
			counter[n].append(k) # ajouter le k associé direct
		else: # sinon, faut d'abord ajouter la clé n puis le k 
			counter[n] = []
			counter[n].append(k)
	print("nombre total d'instances : ", len(folder_content))
	print("répartition des paramètres : ")
	
	for key in counter.keys():
		counter[key].sort() 
		print(key, " -> ", counter[key])


move_inst("inst/")
