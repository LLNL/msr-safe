# Copyright 2011-2023 Lawrence Livermore National Security, LLC and other
# msr-safe Project Developers. See the top-level COPYRIGHT file for
# details.
#
# SPDX-License-Identifier: GPL-2.0-only

import os
import collections
import regex as re

scrap_dir = 'Intel_MSRs'

class MSRMap:
    def __init__(self, directory=scrap_dir):
        self.msrs = {}
        self.file_indices = {}
        self.msr_names = {}
        self.read_all_files(directory)
        self.sorted = False


    def add_msr(self, msr, fname):
        self.sorted = False
        # Treats (msr, arch) as unique (Ik Ik...)
        if msr not in self.msrs:
            self.msrs[msr] = []
        self.msrs[msr].append(fname)

        # Inverted index
        if fname not in self.file_indices:
            self.file_indices[fname] = []
        self.file_indices[fname].append(msr)


    def add_msr(self, msr, fname, name):
        self.sorted = False
        # Treats (msr, arch) as unique (Ik Ik...)
        if msr not in self.msrs:
            self.msrs[msr] = []
        self.msrs[msr].append(fname)

        if msr not in self.msr_names:
            self.msr_names[msr] = {}
        self.msr_names[msr][fname] = name

        # Inverted index
        if fname not in self.file_indices:
            self.file_indices[fname] = []
        self.file_indices[fname].append(msr)


    def get_categories(self, msr):
        if not self.sorted:
            self.sort()
        return self.msrs.get(msr, [])
    

    def get_msrs(self, fname):
        if not self.sorted:
            self.sort()
        return self.file_indices.get(fname, [])


    def read_all_files(self, directory):
        for filename in os.listdir(directory):
            if filename.endswith(".txt"):
                with open(os.path.join(directory, filename), 'r') as f:
                    for line in f:
                        stripped_line = line.lstrip()  # lstrip() removes leading whitespaces
                        if stripped_line and re.match(r'^[0-9A-Fa-f]+H', stripped_line):  # checks if the line starts with a hexadecimal number followed by 'H'
                            parts = stripped_line.split('\t')
                            msr = parts[0].rstrip('H')
                            msr = stripped_line.split('\t')[0].rstrip('H')
                            name = parts[2].strip() if len(parts) > 1 else ''
                            try:
                                msr_int = int(msr, 16)  # Convert hexadecimal string to integer
                                file_name_without_ext = os.path.splitext(filename)[0]
                                self.add_msr(msr_int, file_name_without_ext, name)
                            except ValueError:
                                print('Invalid MSR: ' + msr)
        self.sort()


    def sort(self):
        for key in self.msrs:
            self.msrs[key].sort()
        self.msrs = collections.OrderedDict(sorted(self.msrs.items()))
        for key in self.file_indices:
            self.file_indices[key].sort()
        self.file_indices = collections.OrderedDict(sorted(self.file_indices.items()))
        self.sorted = True

    
    def generate_sorted_unique_msrs_for_file_list(self, architecture_lists):
        msrs = []
        for architecture_list in architecture_lists:
            file_msrs = self.get_msrs(architecture_list)
            for msr in file_msrs:
                msr_name = self.msr_names.get(msr, {}).get(architecture_list, "")
                msrs.append((msr, msr_name))  # Append a tuple containing the MSR and its name
        # Remove duplicates and sort
        msrs = list(set(msrs))
        msrs.sort()
        return msrs


df_dm = {
    "06_2AH": ["2-20", "2-21", "2-22"],
    "06_2DH": ["2-20", "2-23", "2-24"],
    "06_3AH": ["2-20", "2-21", "2-22", "2-25"],
    "06_3CH": ["2-20", "2-21", "2-22", "2-29", "2-30"],
    "06_3DH": ["2-20", "2-21", "2-22", "2-25", "2-29", "2-30", "2-34", "2-35"],
    "06_3EH": ["2-20", "2-24", "2-26", "2-27", "2-28"],
    "06_3FH": ["2-20", "2-29", "2-32", "2-33"],
    "06_45H": ["2-20", "2-21", "2-22", "2-29", "2-30", "2-31", "2-38"],
    "06_46H": ["2-20", "2-21", "2-22", "2-29", "2-30"],
    "06_47H": ["2-20", "2-21", "2-22", "2-25", "2-29", "2-30", "2-34", "2-35"],
    "06_4EH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-40"],
    "06_4FH": ["2-20", "2-21", "2-29", "2-34", "2-36", "2-37", "2-38"],
    "06_55H": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-50"],
    "06_56H": ["2-20", "2-29", "2-34", "2-36", "2-37"],
    "06_5EH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-40"],
    "06_66H": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-40", "2-42", "2-43"],
    "06_6AH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-51"],
    "06_6CH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39"],
    "06_7DH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-44"],
    "06_7EH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-44"],
    "06_8CH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-45"],
    "06_8DH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-45"],
    "06_8EH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-40"],
    "06_8FH": ["2-52"],
    "06_97H": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-46", "2-44", "2-45", "2-47", "2-48", "2-49"],
    "06_9AH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-46", "2-44", "2-45", "2-47", "2-48", "2-49"],
    "06_9EH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-40", "2-41"],
    "06_A5H": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39"],
    "06_A6H": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39"],
    "06_BFH": ["2-20", "2-21", "2-25", "2-29", "2-35", "2-39", "2-46", "2-44", "2-45", "2-47", "2-48", "2-49"]
}


def write_msrs_to_file(msrs, filename, directory='templates'):
    with open(os.path.join(directory, filename), 'w') as f:
        f.write("# MSR # Write Mask # Comment\n")
        for msr, name in msrs:
            try:
                f.write('0x{0:08X} 0x0000000000000000 # "{1}"\n'.format(msr, name))
            except ValueError:
                print('Invalid MSR: ' + msr)
    print('Template written to templates/' + filename)


def main():
    print('Welcome to the MSR allow-list template generator')
    msr_map = MSRMap()

    while True:
        for architecture in msr_map.file_indices:
            print(architecture)
        print('Available architectures:')
        for architecture in df_dm.keys():
            print(architecture)
        print('Enter architecture name to generate template for (A for all):')
        architecture = input()
        if architecture == 'A':
            architectures = df_dm.keys()
            for architecture in architectures:
                msrs = msr_map.generate_sorted_unique_msrs_for_file_list(df_dm[architecture])
                filename = architecture + ".txt"
                write_msrs_to_file(msrs, filename)
            break

        elif architecture not in df_dm:
            print('Architecture not found')
            continue
        else:
            msrs = msr_map.generate_sorted_unique_msrs_for_file_list(df_dm[architecture])
            print('Enter output filename:')
            filename = input()
            with open(os.path.join('templates', filename), 'w') as f:
                write_msrs_to_file(msrs, filename)
            print('Template written to templates/' + filename)

if __name__ == '__main__':
    main()