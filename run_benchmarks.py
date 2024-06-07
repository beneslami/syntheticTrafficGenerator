import os
import shutil
import signal
import subprocess
import sys
sys.path.append("/home/ben/Desktop/modelGenerator/")
import benchlist


def prepare_first(kernels_list, level):
    flag = 0
    for suite in kernels_list.keys():
        for bench, k_list in kernels_list[suite].items():
            for nv in ["NVLink4", "NVLink3", "NVLink2", "NVLink1"]:
                model_path = benchlist.model_eval_path + suite + "/" + bench + "/ring/" + nv + "/4chiplet/data/synthetic/" + level + "/"
                if os.path.exists(model_path):
                    if flag == 0:
                        out = input("Are you sure you want to delete this directory (y/n)? It is " + level + "\n")
                        if out == "y" or out == "yes" or out == "Y":
                            flag = 1
                        else:
                            print("don't worry. Files are still there!")
                            exit(1)
                    elif flag == 1:
                        shutil.rmtree(model_path)
                        os.mkdir(model_path)
                else:
                    os.mkdir(model_path)


if __name__ == "__main__":
    level = "level1"
    kernels_list = benchlist.kernels_list
    try:
        prepare_first(kernels_list, level)
    except:
        pass
    for suite in kernels_list.keys():
        for bench, k_list in kernels_list[suite].items():
            model_path = benchlist.model_eval_path + suite + "/" + bench + "/ideal/4chiplet/" + level + "/"
            for kernel in k_list:
                model_file = model_path + "traffic_model_" + str(kernel) + "_" + level + ".json"
                for nv in ["NVLink4", "NVLink3", "NVLink2", "NVLink1"]:
                    save_path = benchlist.model_eval_path + suite + "/" + bench + "/ring/" + nv + "/4chiplet/data/" + "synthetic/" + level + "/" + str(kernel) + "/"
                    if not os.path.exists(save_path):
                        os.makedirs(save_path)
                    command = ["./src/booksim", "ring", nv.split("k")[1], model_file, save_path]
                    print(" ".join(command))
                    flag = False
                    while not flag:
                        process = subprocess.Popen(command)
                        return_code = process.wait()
                        if return_code == -signal.SIGABRT:
                            shutil.rmtree(save_path)
                            os.makedirs(save_path)
                        else:
                            flag = True