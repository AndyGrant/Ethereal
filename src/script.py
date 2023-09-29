parameters = [
    'float LMRBase    = 0.75;',
    'float LMRDivisor = 2.25;',
    'float LMPNonImpBase   = 2.50;',
    'float LMRNonImpFactor = 0.444;',
    'float LMPImpBase      = 4.00;',
    'float LMRImpFactor    = 0.888;',
    'int LMPDepth = 8;',
    'int WindowDepth   = 5;',
    'int WindowSize    = 10;',
    'int WindowTimerMS = 2500;',
    'int CurrmoveTimerMS = 2500;',
    'int TTResearchMargin = 128;',
    'int BetaPruningDepth = 8;',
    'int BetaMargin = 75;',
    'int AlphaPruningDepth = 5;',
    'int AlphaMargin = 3000;',
    'int NullMovePruningDepth = 2;',
    'int NMPBase              = 4;',
    'int NMPDepthDivisor      = 6;',
    'int NMPEvalCap           = 3;',
    'int NMPEvalDivisor       = 200;',
    'int ProbCutDepth = 5;',
    'int ProbCutMargin = 100;',
    'int IIRDepth = 7;',
    'int SingularDepth = 8;',
    'int SingularTTDepth = 3;',
    'int SingularDoubleMargin = 15;',
    'int FutilityPruningDepth = 8;',
    'int FutilityMarginBase = 92;',
    'int FutilityMarginPerDepth = 59;',
    'int FutilityMarginNoHistory = 158;',
    'int FutilityPruningHistoryLimit[] = { 12000, 6000 };',
    'int ContinuationPruningDepth[] = { 3, 2 };',
    'int ContinuationPruningHistoryLimit[] = { -1000, -2500 };',
    'int LateMovePruningDepth = 8;',
    'int LMRHistoryCap = 2;',
    'int LMRHistoryDivisor = 5000;',
    'int LMRCaptureHistoryDivisor = 5000;',
    'int LMRCaptureBase = 2;',
    'int SEEPruningDepth = 9;',
    'int SEEQuietMargin = -64;',
    'int SEENoisyMargin = -19;',
    'int SEEPieceValues[] = { 100,  450,  450,  675, 1300,    0,    0,    0 };',
    'int QSSeeMargin = 110;',
    'int QSDeltaMargin = 150;',
]


def generate_uci_prompt(name, value):
    return 'printf("option name %s type string default %s\\n");' % (name, str(value))

def generate_set_option(name, index, dtype):

    real_name1 = name if index == None else '%s[%d]' % (name, index)
    real_name2 = name if index == None else '%s_%d' % (name, index)
    conv_func = 'atoi' if dtype == int else 'atof'

    return '\n'.join([
        'if (strStartsWith(str, "setoption name %s value ")) {' % (real_name2),
        '    char *ptr = str + strlen("setoption name %s value ");' % (real_name2),
        '    %s = %s(ptr);' % (real_name1, conv_func),
        '}\n'
    ])

uci_prompts = []

for line in parameters:

    dtype = int if 'int' in line else float
    name = line.split()[1].rstrip('[]')
    is_array = '[]' in line

    if not is_array:
        value = dtype(line.split('=')[1].rstrip(';'))
        uci_prompts.append(generate_uci_prompt(name, value))

        print (generate_set_option(name, None, dtype))

    else:
        for idx, value in enumerate(map(dtype, line.rstrip(' ;}').split('{')[1].split(','))):
            uci_prompts.append(generate_uci_prompt('%s_%d' % (name, idx), value))
            print (generate_set_option(name, idx, dtype))


for prompt in uci_prompts:
    print (prompt)


















