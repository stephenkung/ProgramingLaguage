#### by Liming Gong, 1799174 ####

import re
import os
import argparse
from string import punctuation

parser = argparse.ArgumentParser(description='Top K words')
parser.add_argument('cmd',  default='',type=str, help="cmd parameters")



def get_dict(word_list, cur, w_dict):
	'''
	put {word:frequency} into a dict()
	'''
	if cur==len(word_list): 
		return
	else:
		if word_list[cur] in w_dict.keys():
			w_dict[word_list[cur]] +=1
		else:
			w_dict[word_list[cur]] = 1
		get_dict(word_list, cur+1, w_dict)

		
def get_topk(w_dict, cur, o_dict, k , last_val):
	'''
	get top k from sorted dict()
	'''
	if cur==len(w_dict) or k==0: 
		return
	else:
		if w_dict[cur][1]!=last_val:
			if k>1:
				o_dict[w_dict[cur][0]] = w_dict[cur][1]
				get_topk(w_dict, cur+1, o_dict, k-1, w_dict[cur][1])
			else:
				return
		else:
			o_dict[w_dict[cur][0]] = w_dict[cur][1]
			get_topk(w_dict, cur+1, o_dict, k, w_dict[cur][1])
		
			
	
	
def clean(line, cur, out):
	'''
	lower case + remove punctuation
	'''
	if cur==len(line):
		#print("line out:",out)
		return out
	else:
		ch = line[cur].lower() #lower case
		if ch not in punctuation:
			out += ch
			return clean(line, cur+1, out)
		else:
			return clean(line, cur+1, out)

def has_num(word, cur):
	'''
	check if a word has digit
	'''
	if cur==len(word): 
		return False
	else:
		if word[cur].isdigit():
			return True
		else:
			has_num(word, cur+1)
			
			
def rm_num(w_list, cur, new_list):
	'''
	remove words having digits
	'''
	if cur==len(w_list):
		return
	else:
		if(has_num(w_list[cur],0)):
			rm_num(w_list, cur+1, new_list)
		else:
			new_list.append(w_list[cur])
			rm_num(w_list, cur+1, new_list)


def wirte_topk(o_dict, cur, ofile):
	if cur==len(o_dict):
		return
	else:
		ofile.write(o_dict[cur][0]+" "+str(o_dict[cur][1])+"\n")
		wirte_topk(o_dict, cur+1, ofile)
			
			

def main():
	args = parser.parse_args()
	if args.cmd=="":
		print("Error:please specify the input,k and outout!") 
	else:
		cmd = args.cmd.replace(";","=").replace(" ","").split("=")
		if len(cmd)==6:
			ofile = cmd[-1]
			k = int(cmd[-3])
			ifile = cmd[-5]
			print("input file:",ifile, ",k:", str(k),",output file",ofile)
		else:
			print("Error, too few parameters")
			

	w_list = []
	w_dict = dict()
	o_dict = dict()
	with open(ifile, encoding="utf8") as ii:
		for line in ii:
			temp_out = ""
			temp_new_list = []
			new_line = clean(line, 0 ,temp_out) #lowercase + remove punctuation
			#print("line after clean:", new_line)
			temp_list = new_line.split()
			rm_num(temp_list, 0, temp_new_list) #remove word with digits
			w_list.extend(temp_new_list) #combine all words together
		print("total words:",len(w_list))
		get_dict(w_list, 0, w_dict) #get dictionary with frequency
		print("all dict", w_dict)
		w_dict = sorted(w_dict.items(), key=lambda x: x[1], reverse=True) #sort
		print("sorted dict", w_dict)
		get_topk(w_dict, 0, o_dict, k+1, -1) #get top K
		print("top k dict",o_dict)
		o_dict = sorted(o_dict.items(), key=lambda x: x[1], reverse=True) #sort again
		ofile = open(ofile, 'w')
		wirte_topk(o_dict, 0, ofile)
		

if __name__ == "__main__":
	main()