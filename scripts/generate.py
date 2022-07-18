#!/usr/bin/env python3
import argparse
from pathlib import Path
from tempfile import NamedTemporaryFile
from numpy.random import default_rng
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

def generate_random(rng, N, K):
    s = f'{K} {N} '
    s += (' '.join([str(rng.integers(N)) for x in range(K * N)]))
    s += '\n'
    return s

def generate_cerny(N, K):
    assert K == 2, 'cerny supports only K = 2'
    out = [0] * (K * N)
    ind = lambda n, k: n * K + k
    for i in range(N-1):
        out[ind(i, 0)] = i + 1
        out[ind(i, 1)] = i
    s = f'{K} {N}\n' + ' '.join(map(str, out)) + '\n'
    return s

# rt = n(n-1)/2
def generate_slowlysink(N, K):
    assert K == N-1, 'slowlysink supports only K = N-1'
    
    out = [0] * (K * N)
    ind = lambda n, k: n * K + k
    
    for k in range(N-1):
        for n in range(1, N):
            out[ind(n, k)] = n
        out[ind(k, k)] = k+1
        out[ind(k+1, k)] = k
    out[ind(0, 0)] = 0

    s = f'{K} {N}\n'
    s += ' '.join(map(str, out))
    s += '\n'
    return s

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('K', type=int)
    parser.add_argument('N', type=int)
    parser.add_argument('-o', '--output', type=str,required=True, help='path to the output file')
    parser.add_argument('-t', '--type', choices=['random', 'cerny', 'slowlysink'], default='random')
    parser.add_argument('-s', '--seed', type=int, default=seed_time(), help='seed (int) for the prng')
    parser.add_argument('-c', '--count', type=int, default=1, help='number of automata')
    parser.add_argument('-a', '--append', action='store_const', const=True, help='append at the end if the file exists')
    args = parser.parse_args()
    
    if os.path.exists(args.output) and not args.append:
        parser.error('File already exists, please use -a / --append to add automaton to it')

    print(f'Seed = {args.seed}')
    rng = default_rng(args.seed)

    with open(args.output, 'a+') as file:
        s = ''
        for i in range(args.count):
            if args.type == 'random':
                s += generate_random(rng, args.N, args.K)
            elif args.type == 'cerny':
                s += generate_cerny(args.N, args.K)
            elif args.type == 'slowlysink':
                s += generate_slowlysink(args.N, args.K)
        file.write(s)

if __name__ == '__main__':
    main()
