import tomllib
from pathlib import Path

path = Path("xmake.lua")
workflow_path = Path(".github/workflows/build.yml")
config_path = Path("moddata.toml")
with open(config_path, "rb") as file:
    config = tomllib.load(file)


def change_modname(text, config):
    print("changing modname...")
    return text.replace(
        'local mod_name = "MOD"',
        f'local mod_name = "{config["name"]}"'
    )


def change_license(text, config):
    print("changing license...")
    return text.replace(
        'set_license("MIT")',
        f'set_license("{config["license"]}")'
    )

def add_configs(text, config):
    print("adding configs")
    configs = config.get("configs", {})
    optional = ""
    if configs.get("use_fuck"):
        optional += 'set_config("use-fuck", true)\n'

    return text.replace("--{{ADDITIONAL CONFIGS}}--", optional)

def change_workflowname(wf, config):
    print("changing workflow")
    return wf.replace('MOD_NAME: MOD', f'MOD_NAME: {config["workflowname"]}')

def change_xmakefile(text, config):
    text = change_modname(text, config)
    text = change_license(text, config)
    text = add_configs(text, config)
    return text

def change_workflow(wf, config):
    wf = change_workflowname(wf, config)
    return wf

def main():
    print("working...")
    text = path.read_text(encoding="utf-8")
    wf = workflow_path.read_text(encoding="utf-8")
   
    new_text = change_xmakefile(text, config)
    new_wf = change_workflow(wf, config)
    
    path.write_text(new_text, encoding="utf-8")
    workflow_path.write_text(new_wf, encoding="utf-8")
    print("setup finished")


if __name__ == "__main__":
    main()