//////////////////// BASE DEFINITIONS ////////////////////

// AttachmentOptics slot
class AttachmentOpticsClass {}
AttachmentOpticsClass AttachmentOpticsSource;

class AttachmentOptics : BaseAttachmentType
{
};

//RIS 1913 - pic rail
class AttachmentRIS1913Class {}
AttachmentRIS1913Class AttachmentRIS1913Source;

class AttachmentRIS1913 : AttachmentUnderbarrelRIS1913
{
};

//RIS 1913 - pic rail short
class AttachmentRIS1913ShortClass {}
AttachmentRIS1913ShortClass AttachmentRIS1913ShortSource;

class AttachmentRIS1913Short : AttachmentRIS1913
{
};

// UGL/underbarrel slots
class AttachmentUnderBarrelClass {}
AttachmentUnderBarrelClass AttachmentUnderBarrelSource;

class AttachmentUnderBarrel : BaseAttachmentType
{
};

// Handguard slot
class AttachmentHandGuardClass {}
AttachmentHandGuardClass AttachmentHandGuardSource;

class AttachmentHandGuard : BaseAttachmentType
{
};

// Muzzle slot
class AttachmentMuzzleClass {}
AttachmentMuzzleClass AttachmentMuzzleSource;

class AttachmentMuzzle : BaseAttachmentType
{
};

// Stock slot
class AttachmentStockClass {}
AttachmentStockClass AttachmentStockSource;

class AttachmentStock : BaseAttachmentType
{
};