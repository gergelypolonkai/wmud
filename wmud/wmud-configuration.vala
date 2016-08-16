public errordomain Wmud.ConfigError {
    NOGLOBAL;
}

extern const string WMUD_CONFDIR;

public class Wmud.Configuration : Object {
    private string? filename = null;
    private GLib.KeyFile? key_file;
    private string? admin_email;
    private bool hide_single_race;
    private bool hide_single_class;
    private uint64 house_occupy_time;
    private uint64 minimum_deities;
    private bool clan_wars;
    private uint64 maximum_group_size;
    private bool trainable_abilities;
    private bool reborn;


    public void
    update_from_file(string? filename)
        throws FileError, KeyFileError, ConfigError
    {
        if (filename != null) {
            this.filename = filename;
        } else if (this.filename == null) {
            this.filename = WMUD_CONFDIR + "/wmud.conf";
        }

        key_file = new KeyFile();

        key_file.load_from_file(this.filename, KeyFileFlags.NONE);

        if (!key_file.has_group("global")) {
            throw new ConfigError.NOGLOBAL(
                    "Configuration file (%s) does not contain the required [global] section!",
                    this.filename);
        }

        admin_email = key_file.get_string("global", "admin email");

        hide_single_race = key_file.get_boolean("global", "hide single race");
        hide_single_class = key_file.get_boolean("global", "hide single class");
        house_occupy_time = key_file.get_uint64("global", "house occupy time");
        minimum_deities = key_file.get_uint64("global", "minimum deities");
        clan_wars = key_file.get_boolean("global", "clan wars");
        maximum_group_size = key_file.get_uint64("global", "maximum group size");
        trainable_abilities = key_file.get_boolean("global", "trainable abilities");
        reborn = key_file.get_boolean("global", "reborn");
    }

    public
    Configuration()
    {
        maximum_group_size = 5;
        reborn = true;
    }
}
