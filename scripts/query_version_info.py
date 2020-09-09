from __future__ import print_function

import re
import subprocess
import os
import os.path
import sys
import time
from urllib.parse import unquote

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def to_str(x):
    assert isinstance(x, str) or isinstance(x, bytes)
    if isinstance(x, str):
        return x
    else:
        return x.decode("utf-8")


OUT_FILE = sys.stdout

if "nt" == os.name:
    GIT_PATH_SEARCH = [
        r"C:\Git\bin",
        r"C:\Program Files\Git\bin",
        r"C:\Program Files (x86)\Git\bin"
    ]
else:
    GIT_PATH_SEARCH = [
        "/usr/bin",
        "/usr/local/bin"
    ]


def _add_path_env(value):
    if "nt" == os.name:
        value = '"' + value + '"'
        sep = ";"
    else:
        sep = ":"
    old_path = os.getenv('PATH')
    if old_path is None:
        new_path = value
    else:
        new_path = old_path + sep + value
    os.environ['PATH'] = new_path


for path_variant in GIT_PATH_SEARCH:
    candidate = path_variant + os.path.sep + "git"
    if "nt" == os.name:
        candidate += ".exe"
    if os.path.exists(candidate):
        _add_path_env(path_variant)
        break
    sys.stderr.write("Git executable not found!\n")
    sys.exit(1)


def get_version_information(top_level_dir):
    assert isinstance(top_level_dir, str)
    result = {
        "taggedRelease": False,
        "version": None,
        "hash": None,
        "branch": None,
        "date": get_date(top_level_dir)
    }

    if os.path.exists(top_level_dir + os.path.sep + ".git"):
        hash_tag = ""
        try:
            hash_tag = to_str(subprocess.check_output(
                "git rev-parse --short=12 --verify HEAD",
                shell=True
            )).strip()
        except subprocess.CalledProcessError:
            pass
        if not hash_tag:
            hash_tag = os.environ.get("CI_COMMIT_SHA", "").strip()[:12]
        if hash_tag:
            result["hash"] = hash_tag

        try:
            version_info = subprocess.check_output(
                "git describe --abbrev=0 --tags --exact-match",
                shell=True,
                stderr=subprocess.PIPE
            ).strip()
            result["taggedRelease"] = True
            result["version"] = to_str(version_info)
        except subprocess.CalledProcessError:
            pass

        if result["version"]:
            eprint("get_version_information: tresult=",  result)
            return result

        branch_name = ""
        try:
            branch_name = to_str(subprocess.check_output(
                "git rev-parse --abbrev-ref HEAD",
                shell=True
            )).strip()
        except subprocess.CalledProcessError:
            pass
        if branch_name == "HEAD": # detached HEAD?
            branch_name = os.environ.get("CI_COMMIT_REF_NAME", "").strip()
        if not branch_name:
            branch_name = "NONE"
        result["branch"] = branch_name

    else:
        dir_name = os.path.basename(top_level_dir)
        match = re.match(r"kumir2-(.+)", dir_name)
        version_info = match.group(1)
        if version_info.startswith("2"):
            result["version"] = version_info
            result["taggedRelease"] = True

    #eprint("get_version_information: result=",  result)
    return result


def get_timestamp(top_level_dir):
    assert isinstance(top_level_dir, str)
    if os.path.exists(top_level_dir + os.path.sep + ".git"):
        return to_str(subprocess.check_output(
            "git --no-pager log -1 --pretty=format:%ct",
            shell=True,
            stderr=subprocess.PIPE
        )).strip()
    else:
        return "{}".format(int(time.time()))


def get_date(top_level_dir):
    timestamp = int(get_timestamp(top_level_dir))
    localtime = time.localtime(timestamp)
    assert isinstance(localtime, time.struct_time)
    return "{:04}{:02}{:02}".format(localtime.tm_year, localtime.tm_mon, localtime.tm_mday)


def is_tag(version):
    return version.startswith("2")


def find_suitable_list_file_name(info):
    base = os.getcwd() + os.path.sep + "subdirs-disabled-{}.txt"
    if info["taggedRelease"]:
        name = base.format(info["version"])
    else:
        name = base.format(info["branch"])
    if os.path.exists(name):
        return name
    if info["taggedRelease"]:
        match = re.match(r"(.+)-(alpha|beta|rc|pt|test)[0-9]+", info["version"])
        version_base = match.group(1)
    else:
        version_base = info["branch"]
    if version_base:
        name = base.format(version_base)
        if os.path.exists(name):
            return name


def disabled_modules():
    info = get_version_information(os.getcwd())
    disabled_list_file_name = find_suitable_list_file_name(info)
    disabled_list = []
    if disabled_list_file_name:
        with open(disabled_list_file_name) as source:
            lines = source.read().split('\n')
            for line in lines:
                line = line.strip()
                if line and not line.startswith("#"):
                    disabled_list += [line]
    return disabled_list


def cmake_disabled_modules():
    mods = ["[" + entry + "]" for entry in disabled_modules()]
    if mods:
        return " ".join(mods)
    else:
        return ""


def cmake_version_info():
    info = get_version_information(os.getcwd())
    timestamp = get_timestamp(os.getcwd())
    output = ""
    if info["taggedRelease"]:
        output += "-DGIT_TAG=\"{}\";".format(info["version"])
        output += "-DGIT_BRANCH=\"unknown\";"
    else:
        output += "-DGIT_TAG=\"unknown\";"
        output += "-DGIT_BRANCH=\"{}\";".format(info["branch"])
    output += "-DGIT_HASH=\"{}\";".format(info["hash"])
    output += "-DGIT_TIMESTAMP=\"{}\";".format(timestamp)
    eprint("cmake_version_info: output=",  output)
    return output


def cmake_version_info_tbht():
    info = get_version_information(os.getcwd())
    timestamp = get_timestamp(os.getcwd())
    output_values = []
    if info["taggedRelease"]:
        output_values += [to_str(info["version"])]
        output_values += ["unknown"]
    else:
        output_values += ["unknown"]
        output_values += [to_str(info["branch"])]
    output_values += [to_str(info["hash"])]
    output_values += [to_str(timestamp)]
    output = ";".join(output_values)
    eprint("cmake_version_info_tbht: output=",  output)
    return output


def source_file_name(prefix: str, suffix: str):
    info = get_version_information(os.getcwd())
    if info["taggedRelease"]:
        name = info["version"]
    else:
        name = info["branch"] + "-"
        name += info["hash"] + "-"
        name += info["date"]
    return prefix + name + suffix


def package_bundle_name():
    prefix = ""
    suffix = ""
    nl = ""
    for arg in sys.argv:
        if arg.startswith("--prefix="):
            prefix = unquote(arg[9:].replace('@', '%'))
        elif arg.startswith("--suffix="):
            suffix = unquote(arg[9:].replace('@', '%'))
        elif "--nl" == arg:
            if os.name.startswith("nt"):
                nl = "\r\n"
            else:
                nl = "\n"
    output = source_file_name(prefix, suffix) + nl
    return output


def nsis_include_file():
    info = get_version_information(os.getcwd())
    data = ""
    if info["taggedRelease"]:
        data += "OutFile \"kumir2-" + info["version"] + "-install.exe\"\r\n"
        data += "Name \"Кумир " + info["version"] + "\"\r\n"
        data += "InstallDir \"$PROGRAMFILES\\Kumir-" + info["version"] + "\"\r\n"
        data += "!define VERSION_SUFFIX \"" + info["version"] + "\"\r\n"
    else:
        data += "OutFile \"kumir2-" + info["branch"] + "-"
        data += info["date"] + "-" + info["hash"] + "-install.exe\"\r\n"
        data += "Name \"Кумир 2.x-" + info["branch"] + "\"\r\n"
        data += "InstallDir \"$PROGRAMFILES\\Kumir2x-" + info["branch"] + "\"\r\n"
        data += "!define VERSION_SUFFIX \"" + info["branch"] + "\"\r\n"
    return data


class ChangelogEntry:
    def __init__(self, timestamp: int, author: str, description: str):
        self.timestamp = timestamp
        self.author = author
        self.description = description


def get_changelog(max_count=1000, after=(2015, 5, 1)):
    if os.path.exists(".git"):
        result = []
        command = "git --no-pager log --max-count={} --after={}/{}/{} --no-merges --pretty=format:'%ct;;;%an <%ae>;;;%s'"
        command = command.format(max_count, after[0], after[1], after[2])
        lines = to_str(subprocess.check_output(
            "git describe --abbrev=0 --tags --exact-match",
            shell=True,
            stderr=subprocess.PIPE
        ).strip()).splitlines()
        for line in lines:
            tokens = line.split(";;;")
            entry = ChangelogEntry(int(tokens[0]), tokens[1].strip(), tokens[2].strip())
            result += [entry]
        return result


def main():
    global OUT_FILE
    mode = "package_bundle_name"
    out_file_name = None
    for arg in sys.argv:
        if arg.startswith("--mode="):
            mode = arg[7:]
        elif arg.startswith("--out="):
            out_file_name = arg[6:]
    custom_encoding = False
    if out_file_name:
        if mode.startswith("nsis"):
            open_mode = "wb"
            custom_encoding = "CP1251"
        else:
            open_mode = "w"
        OUT_FILE = open(out_file_name, open_mode)
    if mode in globals():
        eprint("query_version_info.py: mode=",  mode)
        data = globals()[mode]()
        eprint("main: data=",  data)
        if custom_encoding:
            data = data.encode(custom_encoding)
            eprint("main: datae=",  data)
        OUT_FILE.write(data)
    OUT_FILE.close()


if __name__ == "__main__":
    main()
