-- Use folder name to build extension name and tag. Version is specified explicitly.
local ext = get_current_extension_info()

project_ext (ext)

-- Link only those files and folders into the extension target directory
repo_build.prebuild_link {
    { "data", ext.target_dir.."/data" },
    { "docs", ext.target_dir.."/docs" },
    { "spacedata", ext.target_dir.."/spacedata" },
    { "lib", ext.target_dir.."/lib" },
    { "plugins", ext.target_dir.."/plugins" },
}

-- Copy in the schema output libraries and resources
-- repo_build.prebuild_copy
-- {
--     { target_deps.."/usd_plugins/**", ext.target_dir }
-- }