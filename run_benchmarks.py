import os
import shutil


def prepare_first(kernels_list, level):
    flag = 0
    for suite in kernels_list.keys():
        for bench, k_list in kernels_list[suite].items():
            for nv in ["NVLink4", "NVLink3", "NVLink2", "NVLink1"]:
                model_path = path + suite + "/" + bench + "/ring/" + nv + "/4chiplet/data/synthetic/" + level + "/"
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
    path =   "/home/ben/Desktop/benchmarks/"
    kernels_list_orig = {
        "pannotia": {"color-max": [1],
                     "color-maxmin": [2, 3, 4, 5, 7, 8, 9, 10],
                     "pagerank-spmv": [2, 3, 5, 7, 9],
                     "sssp": [3, 6, 9],
                     "fw": [1],
                     "pagerank": [2, 4]
                     },
        "parboil": {"mri-gridding": [1],
                    "spmv": [1]
                    },
        "rodinia": {
            "cfd": [3],
            "gaussian": [2, 4, 6],
        },
    }
    kernels_list = {
        "pannotia": {"color-max": [1],
                     "color-maxmin": [2, 3, 4, 5, 7, 8, 9, 10]
                     },
    }
    try:
        prepare_first(kernels_list, level)
    except:
        pass
    for suite in kernels_list.keys():
        for bench, k_list in kernels_list[suite].items():
            model_path = path + suite + "/" + bench + "/ideal/4chiplet/" + level + "/"
            for kernel in k_list:
                model_file = model_path + "traffic_model_" + str(kernel) + "_" + level + ".json"
                for nv in ["NVLink4", "NVLink3", "NVLink2", "NVLink1"]:
                    save_path = path + suite + "/" + bench + "/ring/" + nv + "/4chiplet/data/" + "synthetic/" + level + "/" + str(kernel) + "/"
                    if not os.path.exists(save_path):
                        os.makedirs(save_path)
                    command = "./src/booksim ring " + nv.split("k")[1] + " " + model_file + " " + save_path
                    os.system(command)