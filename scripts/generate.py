#!/usr/bin/env python3
import argparse
from pathlib import Path
from tempfile import NamedTemporaryFile
from random import randint, seed
from datetime import datetime
import hashlib
import os

def seed_time():
    s = str(datetime.now())
    return int(hashlib.sha1(s.encode("utf-8")).hexdigest(), 16)

def change_dir():
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)

def generate_random(N, K, file):
    file.write(f'{N} {K}\n')
    file.write(' '.join([str(randint(0, N-1)) for x in range(N * K)]))
    file.write('\n')

def generate_hard(N, K, file):
    assert K == 2, 'hard only supports K = 2'
    assert N > 3, 'hard only supports N > 3'

    out = [0] * (N * K)
    ind = lambda n, k: n * K + k
    for i in range(N-1):
        out[ind(i, 0)] = i + 1
        out[ind(i, 1)] = i + 1
    out[ind(N-1, 0)] = 0
    out[ind(N-1, 1)] = 1
    out[ind(1, 1)] = 0
    out[ind(0, 1)] = 3
    out[ind(2, 1)] = 3
    file.write(f'{N} {K}\n')
    file.write(' '.join(map(str, out)))
    file.write('\n')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('N', type=int)
    parser.add_argument('K', type=int)
    parser.add_argument('-o', '--output', type=str,required=True, help='path to the output file')
    parser.add_argument('-t', '--type', choices=['random', 'hard'], default='random')
    parser.add_argument('-s', '--seed', type=int, default=seed_time(), help='seed (int) for the prng')
    parser.add_argument('-c', '--count', type=int, default=1, help='number of automata')
    parser.add_argument('-a', '--append', action='store_const', const=True, help='append at the end if the file exists')
    args = parser.parse_args()
    
    if os.path.exists(args.output) and not args.append:
        parser.error('File already exists, please use -a / --append to add automaton to it')

    print(f'Seed = {args.seed}')
    seed(args.seed)

    with open(args.output, 'a+') as file:
        for i in range(args.count):
            if args.type == 'random':
                generate_random(args.N, args.K, file)
            elif args.type == 'hard':
                generate_hard(args.N, args.K, file)

if __name__ == '__main__':
    main()
