import os
import yaml

def get_parameters():
    with open('config.yaml', 'r') as yml:
            config = yaml.safe_load(yml)
    rx = int(config["rx"])
    ry = int(config["ry"])
    rz = int(config["rz"])
    density = float(config["density"])
    tempreture = float(config["tempreture"])
    A = int(config["A"])
    N = int(config["num_waves"])
    F = float(config["F"])
    step = int(config["step"])
    return rx, ry, rz, density, tempreture, A, N, F, step
        

def generate_input_file(rx, ry, rz, density, tempreture, A, N, F, step, identification):
    with open(f"{identification}/RT.input", "w") as f:
        f.write(f'''units lj
atom_style atomic
boundary p p p
timestep 0.001

read_data ../confs/rho{density}_{rx}x{ry}x{rz}_T{tempreture}_N{N}_A{A}.atoms

mass 1 1.0
mass 2 1.0

pair_style lj/cut 2.5

pair_coeff 1 1 1.0 1.0 2.5
pair_coeff 1 2 1.0 1.0 1.122462048309373
pair_coeff 2 2 1.0 1.0 2.5

group type1 type 1
group type2 type 2

neighbor 0.3 bin
neigh_modify every 20 delay 0 check no

fix 1 all nve
fix 2 type1 addforce 0.0 -{F} 0.0
fix 3 type2 addforce 0.0 {F} 0.0

dump id all atom 2000 {identification}.lammpstrj 
thermo 10000
run {step * 2000}
''')





def generate_job_file(identification):
    with open(f"{identification}/job_rt.sh", "w") as f:
        f.write(f'''#!/bin/bash
#PBS -l nodes=1:ppn=20

cd $PBS_O_WORKDIR

mpirun -np 20 lmp_mpi < RT.input
''')


def generate_job_file_slurm(identification):
    with open(f"{identification}/job_rt.sh", "w") as f:
        f.write(f'''#!/bin/bash
#SBATCH -p F72cpu
#SBATCH -N 72
#SBATCH -n 144

source /home/issp/materiapps/intel/lammps/lammpsvars.sh

srun lammps < RT.input
''')


def generate_yaml(rx, ry, rz, density, tempreture, A, N, F, step, identification):
    with open(f"{identification}/config.yaml", "w") as f:
        f.write(f'''rx: {rx}
ry: {ry}
rz: {rz}
density: {density}
tempreture: {tempreture}
num_waves: {N}
A: {A}
F: {F}
step: {step}
''')



if __name__ == "__main__":
    rx, ry, rz, density, tempreture, A, N, F, step = get_parameters()
    identification = f"rho{density}_{rx}x{ry}x{rz}_T{tempreture}_N{N}_A{A}_F{F}"
    if os.path.isdir(f"{identification}") == False:
        os.mkdir(f"{identification}")
    generate_yaml(rx, ry, rz, density, tempreture, A, N, F, step, identification)
    generate_job_file_slurm(identification)
    generate_input_file(rx, ry, rz, density, tempreture, A, N, F, step, identification)
    print(f"Generated {identification}/")
    
    