import os 
import re 
import subprocess

def run_instance(file_path, elag_begin_percent): 
    cmd = ["/home/periat/LIP6/src/prog", 
           file_path, "0","1","1",
           str(elag_begin_percent)
           ]
    subprocess.run(cmd,check=True)

def execute_insts(): 

    tailles = ["50","100"]
    k = ["20","25","30","35"]
    elag_percent = [0.3, 0.5, 0.7, 0.8, 0.9]

    all_insts = os.listdir("instances/")
    wanted_inst = []

    for inst in all_insts: # tri des instances 
        parts = inst.strip().split("_")
        if parts[0] not in tailles: continue 
        if parts[2] not in k: continue
        wanted_inst.append(inst)

    for inst in wanted_inst: 
        file_path = f"instances/{inst}"
        for e in elag_percent: 
            print("execution de l'instance : ", inst, "pour elag size : ", e)
            run_instance(file_path, e)

execute_insts()        
            